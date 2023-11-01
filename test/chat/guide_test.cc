#include "src/chat/guide.hh"

#include <cassert>

#include <fildesh/string.hh>

#include "llama.h"

#include "src/chat/opt.hh"
#include "src/chat/trajectory.hh"
#include "src/language/vocabulary.hh"

using rendezllama::ChatOptions;
using rendezllama::ChatGuide;
using rendezllama::ChatTrajectory;
using rendezllama::Vocabulary;


static
  void
truncate_detokenize_rolling_to(
    fildesh::ostringstream& oss,
    ChatTrajectory& traj,
    const Vocabulary& vocab)
{
  oss.truncate();
  vocab.detokenize_to(oss, traj.tokens().data() + traj.priming_token_count(),
                      traj.token_count() - traj.priming_token_count());
}


static
  void
the_test(llama_model* model)
{
  FildeshX in[1];
  fildesh::ostringstream oss;
  Vocabulary vocab(model);
  ChatTrajectory traj(vocab.bos_token_id());
  ChatOptions opt;
  ChatGuide guide(vocab, traj, opt);
  bool good;

  // There's nothing to erase, but it still returns true!
  assert(guide.maybe_erase_trailing_message_prefix());
  // Again for good measure.
  assert(guide.maybe_erase_trailing_message_prefix());

  *in = FildeshX_of_strlit("\
    (((chat_prefixes))\n\
     (m (prefix \"A:\") (suffix \"</s>\\n###\\n\"))\n\
     (m (prefix \"B:\"))\n\
     (m (prefix \"C:\"))\n\
     (m (prefix \"D:\") (suffix \"</s>\\n\"))\n\
    )\n\
    (substitution\n\
      (eos_token_alias \"</s>\")\n\
    )\n\
    ");
  good = rendezllama::slurp_sxpb_initialize_options_close_FildeshX(in, opt, "");
  assert(good);
  assert(opt.eos_token_alias == "</s>");
  vocab.assign_substitution(opt.eos_token_alias, vocab.eos_token_id());

  guide.yield_turn();
  truncate_detokenize_rolling_to(oss, traj, vocab);
  assert(oss.view() == "A:");

  guide.yield_turn(2);
  truncate_detokenize_rolling_to(oss, traj, vocab);
  assert(oss.view() == "C:");

  traj.tokenize_append(" Yo.", vocab);
  guide.yield_turn("D: Sup?");
  truncate_detokenize_rolling_to(oss, traj, vocab);
  assert(oss.view() == "C: Yo.\nD: Sup?");

  assert(!guide.maybe_yield_turn());
  traj.tokenize_append("\nOh hi A.\n", vocab);
  assert(!guide.maybe_yield_turn());
  traj.tokenize_append("Sup?</s>", vocab);
  assert(guide.maybe_yield_turn());
  truncate_detokenize_rolling_to(oss, traj, vocab);
  assert(oss.view() == "C: Yo.\nD: Sup?\nOh hi A.\nSup?</s>\nA:");

  assert(!guide.maybe_yield_turn());
  traj.tokenize_append(" Oi!\n</s>", vocab);
  assert(guide.maybe_yield_turn());
  truncate_detokenize_rolling_to(oss, traj, vocab);
  assert(oss.view() == "C: Yo.\nD: Sup?\nOh hi A.\nSup?</s>\nA: Oi!</s>\n###\nB:");

  assert(guide.maybe_erase_trailing_message_prefix());
  truncate_detokenize_rolling_to(oss, traj, vocab);
  assert(oss.view() == "C: Yo.\nD: Sup?\nOh hi A.\nSup?</s>\nA: Oi!</s>\n###\n");

  assert(guide.maybe_erase_trailing_message_suffix());
  truncate_detokenize_rolling_to(oss, traj, vocab);
  assert(oss.view() == "C: Yo.\nD: Sup?\nOh hi A.\nSup?</s>\nA: Oi!");
}


int main(int argc, char** argv)
{
  assert(argc == 2 && "need model filename");

  rendezllama::GlobalScope rendezllama_global_scope;
  llama_model_params model_params = llama_model_default_params();
  model_params.vocab_only = true;
  llama_model* model = llama_load_model_from_file(argv[1], model_params);
  assert(model);

  the_test(model);

  llama_free_model(model);
  return 0;
}

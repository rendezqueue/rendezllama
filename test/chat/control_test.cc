#include "src/chat/control.hh"

#include <cassert>
#include <iostream>

#include <fildesh/fildesh.h>

#include "src/chat/display.hh"
#include "src/chat/guide.hh"
#include "src/chat/opt.hh"
#include "src/chat/trajectory.hh"
#include "src/language/vocabulary.hh"

#include "llama.h"

using rendezllama::ChatControl;
using rendezllama::ChatDisplay;
using rendezllama::ChatGuide;
using rendezllama::ChatOptions;
using rendezllama::ChatTrajectory;
using rendezllama::Vocabulary;

static void test_accept_generated_token(Vocabulary& vocab) {
  ChatOptions opt;
  opt.sentence_token_limit = 5;

  ChatTrajectory traj(vocab.bos_token_id());
  ChatGuide guide(vocab, traj, opt);
  ChatDisplay display;
  display.out_ = open_FildeshOF("/dev/null");

  ChatControl control;
  std::string matched_antiprompt;

  // Test basic generation step.
  control.accept_generated_token(guide, display, traj, matched_antiprompt, opt, vocab);
  assert(control.has_input_mode_on());

  control.set_input_mode_on(false);
  control.accept_generated_token(guide, display, traj, matched_antiprompt, opt, vocab);
  assert(!control.has_input_mode_on());

  // Test sentence token limit.
  for (unsigned i = 0; i < 4; ++i) {
    control.accept_generated_token(guide, display, traj, matched_antiprompt, opt, vocab);
  }

  assert(control.has_input_mode_on());

  // Test antiprompt match.
  control.set_input_mode_on(false);
  matched_antiprompt = "User:";
  control.accept_generated_token(guide, display, traj, matched_antiprompt, opt, vocab);
  assert(!control.has_input_mode_on());

  // Test byte limit.
  control.reset_textgen_byte_limit(10);
  control.increment_textgen_byte_count(10);
  control.accept_generated_token(guide, display, traj, matched_antiprompt, opt, vocab);
  assert(control.has_input_mode_on());
}

int main(int argc, char** argv) {
  assert(argc == 2 && "need model filename");

  rendezllama::GlobalScope rendezllama_global_scope;
  llama_model_params model_params = llama_model_default_params();
  model_params.vocab_only = true;
  llama_model* model = llama_model_load_from_file(argv[1], model_params);
  assert(model);
  Vocabulary vocab(model);

  test_accept_generated_token(vocab);

  llama_model_free(model);
  return 0;
}

#include "src/chat/trajectory.hh"

#include <cassert>

#include <fildesh/fildesh.h>

#include "llama.h"

#include "src/language/vocabulary.hh"

using rendezllama::ChatTrajectory;
using rendezllama::Vocabulary;


static
  void
basic_test()
{
  ChatTrajectory traj(0);
  assert(traj.mirostat_mu() == 0.0f);
  assert(traj.last_message_prefix_id_at(0) == traj.not_a_message_prefix_id());
  for (unsigned i = 1; i < 100; ++i) {
    traj.push_back(i);
    assert(traj.mirostat_mu_at(i) == traj.mirostat_mu());
    assert(traj.mirostat_mu_at(i) == traj.mirostat_mu_at(i-1));
    traj.mirostat_mu_at(i) = i;
    assert(traj.mirostat_mu() == i);
  }
  for (unsigned i = 1; i < 10; ++i) {
    traj.assign_range_message_prefix_id(i, 10*i, 11*i+1);
    assert(traj.message_prefix_id_ == i);
  }

  traj.assign_range_message_prefix_id(0, 0, 1);
  assert(traj.last_message_prefix_id_at(0) == 0);
  assert(traj.message_prefix_id_ == 9);
  assert(traj.token_count() == 100);

  assert(traj.find_token_at(0, 1) == 1);
  assert(traj.find_token_at(1, 1) == 1);
  assert(traj.find_token_at(2, 1) == 100);
  assert(traj.rfind_token_at(0, 1) == 100);
  assert(traj.rfind_token_at(1, 1) == 1);
  assert(traj.rfind_token_at(2, 1) == 1);
  assert(traj.rfind_token_at(100, 1) == 1);
  assert(traj.rfind_token_at(0, 0) == 0);

  assert(traj.rfind_message_prefix_at(49) == 44);
  assert(traj.rfind_message_prefix_begin_at(49) == 40);
  assert(traj.rfind_last_message_prefix_end_at(49) == 45);
  assert(traj.rfind_last_message_prefix_end_at(50) == 45);
  assert(traj.rfind_last_message_prefix_end_at(55) == 45);
  assert(traj.rfind_last_message_prefix_end_at(56) == 56);
  assert(traj.last_message_prefix_id_at(49) == 4);
  assert(traj.last_message_prefix_id_at(50) == 4);
  assert(traj.last_message_prefix_id_at(55) == 4);
  assert(traj.last_message_prefix_id_at(56) == 5);

  // Delete last 10.
  traj.erase_all_at(90);
  assert(traj.token_count() == 90);
  assert(traj.message_prefix_id_ == 8);

  // These token counts are incremented by other code,
  // but we expect ChatTrajectory to decrease them during rollforget.
  traj.context_token_count_ = 80;
  traj.display_token_count_ = 70;

  // Delete [40..49].
  traj.erase_range(40, 50);
  assert(traj.token_count() == 80);
  assert(traj.context_token_count_ == 40);
  assert(traj.display_token_count_ == 60);

  assert(traj.message_prefix_id_ == 8);
  assert(traj.rfind_last_message_prefix_end_at(49) == 46);
  assert(traj.rfind_last_message_prefix_end_at(50) == 46);
  assert(traj.rfind_last_message_prefix_end_at(56) == 46);
  assert(traj.rfind_last_message_prefix_end_at(57) == 57);

  // Restore [40..49].
  traj.insert_all_at(
      40,
      std::vector<Vocabulary::Token_id>{40, 41, 42, 43, 44, 45, 46, 47, 48, 49});
  assert(traj.token_count() == 90);
  assert(traj.message_prefix_id_ == 8);
  assert(traj.rfind_last_message_prefix_end_at(49) == 34);
  traj.assign_range_message_prefix_id(4, 40, 45);
  assert(traj.rfind_last_message_prefix_end_at(49) == 45);
}


static
  void
rollforget_test(const char* model_filename)
{
  llama_model_params model_params = llama_model_default_params();
  model_params.vocab_only = true;
  llama_model* model = llama_load_model_from_file(model_filename, model_params);
  assert(model);
  llama_context_params ctx_params = llama_context_default_params();
  llama_context* ctx = llama_new_context_with_model(model, ctx_params);
  assert(ctx);

  const rendezllama::Vocabulary vocabulary(ctx);
  ChatTrajectory traj(vocabulary.bos_token_id());

  FildeshO transcript_out[1] = {DEFAULT_FildeshO};
  // `traj` takes ownership and will free the memory.
  traj.transcript_out_ = transcript_out;

  traj.tokenize_append(
      " Transcript of a conversation between User and their Code.\n"
      "\n"
      "### Transcript Continuation\n",
      vocabulary);
  traj.priming_token_count_ = traj.token_count();

  traj.tokenize_append_message_prefix(0, "User:", vocabulary);
  traj.tokenize_append(" Tell me all your bugs!\n", vocabulary);
  traj.tokenize_append_message_prefix(1, "Code:", vocabulary);
  traj.tokenize_append(" I cannot.\n", vocabulary);
  const unsigned expect_forget_count = (
      traj.token_count() - traj.priming_token_count_);
  traj.tokenize_append_message_prefix(0, "User:", vocabulary);
  traj.tokenize_append(" Why not?\n", vocabulary);
  traj.tokenize_append_message_prefix(1, "Code:", vocabulary);
  traj.tokenize_append(
      " They are not enumerable, but I can give a sample."
      " (1) Infinite loop on line 20."
      " (2) Off-by-one on line 21."
      " (3) Off-by-two on line 21."
      " (4) Segmentation fault\n",
      vocabulary);
  traj.tokenize_append_message_prefix(0, "User:", vocabulary);
  traj.tokenize_append(" wtf\n", vocabulary);

  const unsigned old_token_count = traj.token_count();
  traj.rollforget(traj.priming_token_count_ + expect_forget_count, vocabulary);
  assert(traj.token_count() < old_token_count);
  assert(traj.token_count() == old_token_count - expect_forget_count);

  assert(traj.transcript_out_->size > 0);

  llama_free(ctx);
  llama_free_model(model);
}


int main(int argc, char** argv)
{
  assert(argc == 2 && "need model filename");
  basic_test();
  rendezllama::GlobalScope rendezllama_global_scope;
  rollforget_test(argv[1]);
  return 0;
}

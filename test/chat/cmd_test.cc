#include "src/chat/cmd.hh"

#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>

#include <fildesh/fildesh.h>

#include "src/chat/opt.hh"
#include "src/chat/trajectory.hh"
#include "src/language/vocabulary.hh"

#include "llama.h"

using rendezllama::ChatOptions;
using rendezllama::ChatTrajectory;
using rendezllama::Vocabulary;

static void test_maybe_do_back_command(const Vocabulary& vocab) {
  ChatOptions opt;
  ChatTrajectory traj(vocab.bos_token_id());

  traj.tokenize_append("One Two Three", vocab);
  traj.priming_token_count_ = 1;

  std::stringstream out;

  const char* s = "b 1";
  FildeshX in_struct = FildeshX_of_bytestring((const unsigned char*)s, strlen(s));
  FildeshX* in = &in_struct;

  unsigned count_before = traj.token_count();
  bool result = rendezllama::maybe_do_back_command(traj, in, out, vocab, opt);
  assert(result);
  assert(traj.token_count() == count_before - 1);

  s = "b 2";
  in_struct = FildeshX_of_bytestring((const unsigned char*)s, strlen(s));
  in = &in_struct;
  count_before = traj.token_count();
  result = rendezllama::maybe_do_back_command(traj, in, out, vocab, opt);
  assert(result);
  assert(traj.token_count() == count_before - 2);

  s = "not a command";
  in_struct = FildeshX_of_bytestring((const unsigned char*)s, strlen(s));
  in = &in_struct;
  result = rendezllama::maybe_do_back_command(traj, in, out, vocab, opt);
  assert(!result);
}

static void test_maybe_do_delete_command(const Vocabulary& vocab) {
  ChatOptions opt;
  ChatTrajectory traj(vocab.bos_token_id());
  traj.priming_token_count_ = 1;

  traj.tokenize_append_message_prefix(0, "User:", vocab);
  traj.tokenize_append(" Hello", vocab);
  traj.tokenize_append_message_suffix("\n", vocab);
  traj.tokenize_append_message_prefix(1, "Code:", vocab);
  traj.tokenize_append(" Hi", vocab);

  unsigned count_before = traj.token_count();

  const char* s = "d";
  FildeshX in_struct = FildeshX_of_bytestring((const unsigned char*)s, strlen(s));
  FildeshX* in = &in_struct;

  bool result = rendezllama::maybe_do_delete_command(in, traj, opt);
  assert(result);
  assert(traj.token_count() < count_before);
}

static void test_maybe_do_delete_inline_command(const Vocabulary& vocab) {
  ChatOptions opt;
  ChatTrajectory traj(vocab.bos_token_id());
  traj.priming_token_count_ = 1;

  traj.tokenize_append("Line1\nLine2\nLine3", vocab);

  const char* s = "D";
  FildeshX in_struct = FildeshX_of_bytestring((const unsigned char*)s, strlen(s));
  FildeshX* in = &in_struct;

  bool result = rendezllama::maybe_do_delete_inline_command(in, traj, vocab, opt);
  assert(result);
}

static void test_maybe_do_head_command(const Vocabulary& vocab) {
  ChatOptions opt;
  ChatTrajectory traj(vocab.bos_token_id());
  traj.tokenize_append("Line1\nLine2\nLine3\n", vocab);
  traj.priming_token_count_ = 1;

  std::stringstream out;
  const char* s = "head 2";
  FildeshX in_struct = FildeshX_of_bytestring((const unsigned char*)s, strlen(s));
  FildeshX* in = &in_struct;

  bool result = rendezllama::maybe_do_head_command(in, out, vocab, traj, opt);
  assert(result);
  std::string output = out.str();
  assert(output.find("Line1") != std::string::npos);
  assert(output.find("Line2") != std::string::npos);
}

static void test_maybe_do_print_tokens_command(const Vocabulary& vocab) {
  ChatOptions opt;
  ChatTrajectory traj(vocab.bos_token_id());
  traj.tokenize_append("A B", vocab);
  traj.priming_token_count_ = 1;

  std::stringstream out;
  const char* s = "pt 2";
  FildeshX in_struct = FildeshX_of_bytestring((const unsigned char*)s, strlen(s));
  FildeshX* in = &in_struct;

  bool result = rendezllama::maybe_do_print_tokens_command(in, out, vocab, traj, opt);
  assert(result);
  assert(out.str().length() > 0);
}

static void test_maybe_do_regen_command(const Vocabulary& vocab) {
  ChatOptions opt;
  ChatTrajectory traj(vocab.bos_token_id());
  traj.priming_token_count_ = 1;

  traj.tokenize_append_message_prefix(0, "User:", vocab);
  traj.tokenize_append(" Hi", vocab);
  traj.tokenize_append_message_suffix("\n", vocab);
  traj.tokenize_append_message_prefix(1, "Code:", vocab);
  traj.tokenize_append(" Hello", vocab);

  const char* s = "r";
  FildeshX in_struct = FildeshX_of_bytestring((const unsigned char*)s, strlen(s));
  FildeshX* in = &in_struct;

  bool result = rendezllama::maybe_do_regen_command(in, traj, opt);
  assert(result);
}

static void test_maybe_do_regen_inline_command(const Vocabulary& vocab) {
  ChatOptions opt;
  ChatTrajectory traj(vocab.bos_token_id());
  traj.priming_token_count_ = 1;

  traj.tokenize_append_message_prefix(0, "User:", vocab);
  traj.tokenize_append(" Hi", vocab);

  const char* s = "R";
  FildeshX in_struct = FildeshX_of_bytestring((const unsigned char*)s, strlen(s));
  FildeshX* in = &in_struct;

  bool result = rendezllama::maybe_do_regen_inline_command(in, traj, opt);
  assert(result);
}

static void test_maybe_do_tail_command(const Vocabulary& vocab) {
  ChatOptions opt;
  ChatTrajectory traj(vocab.bos_token_id());
  // Removed trailing newline so tail 1 gets Line3.
  traj.tokenize_append("Line1\nLine2\nLine3", vocab);
  traj.priming_token_count_ = 1;

  std::stringstream out;
  const char* s = "tail 1";
  FildeshX in_struct = FildeshX_of_bytestring((const unsigned char*)s, strlen(s));
  FildeshX* in = &in_struct;

  bool result = rendezllama::maybe_do_tail_command(in, out, vocab, traj, opt);
  assert(result);
  std::string output = out.str();
  assert(output.find("Line3") != std::string::npos);
}

static void test_maybe_parse_yield_command() {
  ChatOptions opt;
  opt.confidant = "Confidant";

  std::string ret_buffer;
  const char* s = "yield";
  FildeshX in_struct = FildeshX_of_bytestring((const unsigned char*)s, strlen(s));
  FildeshX* in = &in_struct;

  bool result = rendezllama::maybe_parse_yield_command(ret_buffer, in, opt);
  assert(result);
  assert(ret_buffer.find("Confidant:") != std::string::npos);

  s = "y extra";
  in_struct = FildeshX_of_bytestring((const unsigned char*)s, strlen(s));
  in = &in_struct;
  result = rendezllama::maybe_parse_yield_command(ret_buffer, in, opt);
  assert(result);
  assert(ret_buffer.find("extra") != std::string::npos);
}

int main(int argc, char** argv) {
  assert(argc == 2 && "need model filename");

  rendezllama::GlobalScope rendezllama_global_scope;
  llama_model_params model_params = llama_model_default_params();
  model_params.vocab_only = true;
  llama_model* model = llama_model_load_from_file(argv[1], model_params);
  assert(model);
  Vocabulary vocab(model);

  test_maybe_do_back_command(vocab);
  test_maybe_do_delete_command(vocab);
  test_maybe_do_delete_inline_command(vocab);
  test_maybe_do_head_command(vocab);
  test_maybe_do_print_tokens_command(vocab);
  test_maybe_do_regen_command(vocab);
  test_maybe_do_regen_inline_command(vocab);
  test_maybe_do_tail_command(vocab);
  test_maybe_parse_yield_command();

  llama_model_free(model);
  return 0;
}

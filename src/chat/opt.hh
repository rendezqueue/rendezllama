#ifndef RENDEZLLAMA_OPT_HH_
#define RENDEZLLAMA_OPT_HH_

#include <string>
#include <ostream>
#include <set>
#include <vector>

#include "src/language/language_schema.hh"

struct FildeshX;
struct FildeshSxprotoField;

namespace rendezllama {

struct ChatMessageOpt {
  std::string prefix;
  std::string given_prefix;
  std::string suffix;
  std::string given_suffix;
};

struct ChatOptions {

  std::string protagonist;
  std::string confidant;
  language::Substitution substitution;
  std::vector<ChatMessageOpt> message_opts;
  std::string model_filename;
  std::string lora_filename;
  std::string transcript_sibling_filename;
  std::string transcript_filename;

  std::string priming_prompt;
  std::string rolling_prompt;
  std::string answer_prompt;
  // Match original LLaMA tokenizer behavior by starting with a space.
  bool bos_token_on = true;
  bool startspace_on = true;
  // Add space before all lines.
  bool linespace_on = false;

  char command_prefix_char = '/';
  const char command_delim_chars[5] = ":=! ";

  unsigned thread_count = 1;
  unsigned batch_thread_count = 0;
  unsigned sentence_limit = 0;
  unsigned sentence_token_limit = 0;

  unsigned model_token_limit = 0;  // Default derived from model.
  unsigned context_token_limit = 0;  // Defaults to model_token_limit.
  unsigned batch_count = 512;
  bool mlock_on = false;
  bool mmap_on = true;
  bool coprocess_mode_on = false;
  std::set<std::string> sentence_terminals = {"!", ".", "?", "…"};
  std::set<std::string> antiprompts;
  // Can't set these yet.
  bool verbose_prompt = false;

  inference::InferVia infer_via;
};

void
print_options(std::ostream& out, const ChatOptions& opt);
int
parse_options(ChatOptions& opt, int argc, char** argv);
bool
slurp_sxpb_options_close_FildeshX(
    FildeshX* in,
    rendezllama::ChatOptions& opt,
    const FildeshSxprotoField* schema,
    const std::string& filename);
bool
slurp_sxpb_initialize_options_close_FildeshX(
    FildeshX* in,
    rendezllama::ChatOptions& opt,
    const std::string& filename);
bool
slurp_sxpb_dynamic_options_close_FildeshX(
    FildeshX* in,
    rendezllama::ChatOptions& opt);


}  // namespace rendezllama
#endif

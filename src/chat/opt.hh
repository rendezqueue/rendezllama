#ifndef RENDEZLLAMA_OPT_HH_
#define RENDEZLLAMA_OPT_HH_

#include <string>
#include <ostream>
#include <vector>

#include "llama.h"

struct FildeshX;

namespace rendezllama {

struct ChatOptions {

  std::string protagonist;
  std::string confidant;
  std::string model_filename;
  std::string transcript_filename;

  std::string priming_prompt;
  std::string rolling_prompt;
  // Match original LLaMA tokenizer behavior by starting ith a space.
  bool bos_token_on = true;
  bool startspace_on = true;
  // Add space before all lines.
  bool linespace_on = false;

  char command_prefix_char = '/';
  const char command_delim_chars[5] = ":=! ";

  int thread_count = 1;
  int sequent_token_limit = 50;
  int top_k = 1000;
  float top_p = 0.95;
  float temp = 0.7;
  float repeat_penalty = 1.2;
  int repeat_last_count = 20;
  int context_token_limit = 2048;
  int batch_count = 8;
  int seed;
  // Can't set these yet.
  std::vector<std::string> antiprompts;
  bool verbose_prompt = true;

  int priming_token_count = 0;
  bool mlock_on = false;
};

int
parse_options(ChatOptions& opt, int argc, char** argv);
bool
maybe_parse_option_command(
    rendezllama::ChatOptions& opt,
    FildeshX* in,
    std::ostream& eout);
struct llama_context*
make_llama_context(const ChatOptions& opt);
void
print_initialization(
    std::ostream& out,
    struct llama_context* ctx,
    const ChatOptions& opt,
    const std::vector<llama_token>& tokens);

}  // namespace rendezllama
#endif

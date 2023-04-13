#ifndef RENDEZLLAMA_OPT_HH_
#define RENDEZLLAMA_OPT_HH_

#include <string>
#include <ostream>
#include <vector>

#include "llama.h"


namespace rendezllama {

struct ChatOptions {

  std::string protagonist;
  std::string confidant;
  // Match original LLaMA tokenizer behavior by starting ith a space.
  std::string priming_prompt = " ";
  std::string rolling_prompt;
  std::string model_filename;
  int thread_count = 1;
  int sentence_token_limit = 2048;
  // Can't set these yet.
  int top_k = 1000;
  float top_p = 0.95;
  float temp = 0.7;
  float repeat_penalty = 1.2;
  int repeat_last_count = 2048;
  int context_token_limit = 2048;
  int batch_count = 8;
  int seed;
  std::vector<std::string> antiprompts;
  bool verbose_prompt = true;

  int priming_token_count = 0;
  bool mlock_on = false;
};

int
parse_options(ChatOptions& opt, int argc, char** argv);
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

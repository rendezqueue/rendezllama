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
  std::string template_protagonist;
  std::string template_confidant;
  std::string model_filename;
  std::string lora_filename;
  std::string lora_base_model_filename;
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
  unsigned sentence_limit = 3;
  unsigned sentence_token_limit = 50;
  unsigned top_k = 1000;
  float top_p = 0.95;
  float temp = 0.7;
  float tfs_z = 1.0;
  float typical_p = 1.0;
  float frequency_penalty = 0.0;
  float presence_penalty = 0.0;
  float repeat_penalty = 1.2;
  unsigned repeat_last_count = 20;
  unsigned mirostat_sampling = 0;
  float mirostat_tau = 5.0;
  float mirostat_eta = 0.1;
  unsigned context_token_limit = 2048;
  unsigned batch_count = 8;
  int seed;
  bool mlock_on = false;
  bool mmap_on = true;
  // Can't set these yet.
  std::vector<std::string> antiprompts;
  bool verbose_prompt = false;
};

void
print_options(std::ostream& out, const ChatOptions& opt);
int
parse_options(ChatOptions& opt, int argc, char** argv);
bool
maybe_parse_option_command(
    rendezllama::ChatOptions& opt,
    FildeshX* in,
    std::ostream& eout);
struct llama_context*
make_llama_context(const ChatOptions& opt);

}  // namespace rendezllama
#endif

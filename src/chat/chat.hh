#ifndef RENDEZLLAMA_CHAT_HH_
#define RENDEZLLAMA_CHAT_HH_
#include <ostream>
#include <string>
#include <vector>

#include "llama.h"

namespace rendezllama {

struct ChatOptions;

const std::string&
antiprompt_suffix(const std::string& text,
                  const std::vector<std::string>& antiprompts);
void
augment_chat_input(std::string& s,
                   const std::string& matched_antiprompt,
                   const ChatOptions& opt);

llama_token
generate_next_token(struct llama_context* ctx,
                    const std::vector<llama_token>& extra_penalized_tokens,
                    const std::vector<llama_token>& tokens,
                    const ChatOptions& opt);
unsigned
commit_to_context(struct llama_context* ctx,
                  std::ostream& out,
                  std::vector<llama_token>& chat_tokens,
                  unsigned context_token_count,
                  const ChatOptions& opt);

}  // namespace rendezllama
#endif

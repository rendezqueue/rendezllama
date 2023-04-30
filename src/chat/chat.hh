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
                   bool& prevent_subsequent_newline,
                   const std::string& matched_antiprompt,
                   const ChatOptions& opt);

llama_token
generate_next_token(struct llama_context* ctx,
                    bool preventing_newline,
                    const std::vector<llama_token>& extra_penalized_tokens,
                    const std::vector<llama_token>& tokens,
                    const ChatOptions& opt);
unsigned
commit_to_context(struct llama_context* ctx,
                  std::ostream& out,
                  std::ostream& transcript_out,
                  std::vector<llama_token>& chat_tokens,
                  unsigned context_token_count,
                  const ChatOptions& opt);
unsigned
maybe_insert_answer_prompt(
    std::vector<llama_token>& chat_tokens,
    struct llama_context* ctx,
    unsigned answer_prompt_offset,
    const std::vector<llama_token>& answer_prompt_tokens);
void
maybe_remove_answer_prompt(
    std::vector<llama_token>& chat_tokens,
    unsigned& context_token_count,
    unsigned& answer_prompt_offset,
    const std::vector<llama_token>& answer_prompt_tokens,
    bool inputting);

}  // namespace rendezllama
#endif

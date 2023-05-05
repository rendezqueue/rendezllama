#ifndef RENDEZLLAMA_CHAT_HH_
#define RENDEZLLAMA_CHAT_HH_
#include <ostream>
#include <string>
#include <vector>

#include "llama.h"

namespace rendezllama {

struct ChatOptions;
struct ChatTrajectory;

const std::string&
antiprompt_suffix(const std::string& text,
                  const std::vector<std::string>& antiprompts);
void
augment_chat_input(std::string& s,
                   bool& prevent_subsequent_newline,
                   const std::string& matched_antiprompt,
                   const ChatOptions& opt);


void
tokenize_extend(
    rendezllama::ChatTrajectory& chat_traj,
    llama_context* ctx, const std::string& s);
void
generate_next_token(
    ChatTrajectory& chat_traj,
    struct llama_context* ctx,
    bool preventing_newline,
    const std::vector<llama_token>& extra_penalized_tokens,
    const ChatOptions& opt);
bool
commit_to_context(struct llama_context* ctx,
                  std::ostream& out,
                  rendezllama::ChatTrajectory& chat_traj,
                  const ChatOptions& opt);
unsigned
maybe_insert_answer_prompt(
    ChatTrajectory& chat_traj,
    struct llama_context* ctx,
    unsigned answer_prompt_offset,
    const std::vector<llama_token>& answer_prompt_tokens);
void
maybe_remove_answer_prompt(
    rendezllama::ChatTrajectory& chat_traj,
    unsigned& answer_prompt_offset,
    const std::vector<llama_token>& answer_prompt_tokens,
    bool inputting);

}  // namespace rendezllama
#endif

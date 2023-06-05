#ifndef RENDEZLLAMA_CHAT_HH_
#define RENDEZLLAMA_CHAT_HH_
#include <ostream>
#include <string>
#include <vector>

#include "llama.h"

namespace rendezllama {

struct ChatOptions;
class ChatDisplay;
class ChatTrajectory;

const std::string&
antiprompt_suffix(const std::string& text,
                  const std::vector<std::string>& antiprompts);
void
augment_tokenize_chat_input(
    ChatTrajectory& chat_traj,
    llama_context* ctx,
    bool& prevent_subsequent_newline,
    std::string s,
    const std::string& matched_antiprompt,
    const ChatOptions& opt);


struct llama_context*
make_llama_context(const ChatOptions& opt);
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
                  ChatDisplay& chat_disp,
                  ChatTrajectory& chat_traj,
                  const ChatOptions& opt);

}  // namespace rendezllama
#endif

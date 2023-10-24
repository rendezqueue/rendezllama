#ifndef RENDEZLLAMA_CHAT_HH_
#define RENDEZLLAMA_CHAT_HH_
#include <ostream>
#include <string>
#include <tuple>
#include <vector>

#include "llama.h"

namespace rendezllama {

struct ChatOptions;
class ChatDisplay;
class ChatTrajectory;
class Vocabulary;

const std::string&
antiprompt_suffix(
    std::string_view text,
    const std::vector<std::string>& antiprompts);
bool
eom_token_check(
    const Vocabulary& vocabulary,
    llama_token token_id,
    const ChatOptions& opt,
    const ChatTrajectory& chat_traj);
void
augment_tokenize_chat_input(
    ChatTrajectory& chat_traj,
    bool& prevent_subsequent_newline,
    std::string s,
    const std::string& matched_antiprompt,
    const Vocabulary& vocabulary,
    const ChatOptions& opt);


std::tuple<struct llama_model*, struct llama_context*>
make_llama_context(ChatOptions& opt);
void
generate_next_token(
    ChatTrajectory& chat_traj,
    struct llama_context* ctx,
    bool preventing_newline,
    const std::vector<llama_token>& extra_penalized_tokens,
    const Vocabulary& vocabulary,
    const ChatOptions& opt);
bool
commit_to_context(struct llama_context* ctx,
                  ChatDisplay& chat_disp,
                  ChatTrajectory& chat_traj,
                  const Vocabulary& vocabulary,
                  const ChatOptions& opt);

}  // namespace rendezllama
#endif

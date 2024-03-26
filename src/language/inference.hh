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
class ChatGuide;
class ChatTrajectory;
class Vocabulary;

const std::string&
antiprompt_suffix(
    std::string_view text,
    const std::vector<std::string>& antiprompts);
void
augment_tokenize_chat_input(
    ChatGuide& chat_guide,
    ChatTrajectory& chat_traj,
    bool& prevent_subsequent_newline,
    std::string s,
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

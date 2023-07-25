#ifndef RENDEZLLAMA_CHAT_DISPLAY_HH_
#define RENDEZLLAMA_CHAT_DISPLAY_HH_
#include "src/chat/trajectory.hh"

namespace rendezllama {

class ChatDisplay {
 public:
  typedef ChatTrajectory::size_type size_type;
  typedef ChatTrajectory::Token_id Token_id;

 public:
  ChatDisplay() {}
  ~ChatDisplay();

  static
    const char*
  displaystring(
      ChatTrajectory::Token_id token_id,
      const struct llama_context* ctx);

  void show_new(size_type end,
                ChatTrajectory& chat_traj,
                const struct llama_context* ctx);
  void show_new(ChatTrajectory& chat_traj,
                const struct llama_context* ctx);
  void maybe_insert_answer_prompt(ChatTrajectory& chat_traj, struct llama_context* ctx);
  void maybe_remove_answer_prompt(ChatTrajectory& chat_traj, bool inputting);

 public:
  FildeshO* out_ = nullptr;
  unsigned answer_prompt_offset_ = 0;
  std::vector<Token_id> answer_prompt_tokens_;
};

}  // namespace rendezllama
#endif


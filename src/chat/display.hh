#ifndef RENDEZLLAMA_CHAT_DISPLAY_HH_
#define RENDEZLLAMA_CHAT_DISPLAY_HH_
#include "src/chat/trajectory.hh"

namespace rendezllama {

class ChatDisplay {
 public:
  ChatDisplay() {}
  ~ChatDisplay();

  void
  displaystring_to(
      FildeshO* out,
      Vocabulary::Token_id token_id,
      const Vocabulary& vocabulary) const;

  void show_new(ChatTrajectory::size_type end,
                ChatTrajectory& chat_traj,
                const Vocabulary& vocabulary);
  void show_new(ChatTrajectory& chat_traj,
                const Vocabulary& vocabulary);
  void maybe_insert_answer_prompt(ChatTrajectory& chat_traj,
                                  const Vocabulary& vocabulary);
  void maybe_remove_answer_prompt(ChatTrajectory& chat_traj, bool inputting);

 public:
  FildeshO* out_ = nullptr;
  unsigned answer_prompt_offset_ = 0;
  std::vector<Vocabulary::Token_id> answer_prompt_tokens_;
};

}  // namespace rendezllama
#endif

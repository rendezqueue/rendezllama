#ifndef RENDEZLLAMA_CHAT_GUIDE_HH_
#define RENDEZLLAMA_CHAT_GUIDE_HH_
#include <string>

namespace rendezllama {

struct ChatOptions;
class ChatDisplay;
class ChatTrajectory;
class Vocabulary;

class ChatGuide {
 public:
  explicit ChatGuide(Vocabulary& vocab, ChatTrajectory& traj, ChatOptions& opt)
    : vocab_(vocab), traj_(traj), opt_(opt)
  {}

  bool maybe_erase_trailing_message_prefix();
  bool maybe_erase_trailing_message_suffix();

  void begin_turn(unsigned turn_index);
  void end_turn();
  void yield_turn(unsigned turn_index);
  void yield_turn(std::string_view prefix);
  void yield_turn();
  bool maybe_yield_turn();

 private:
  Vocabulary& vocab_;
  ChatTrajectory& traj_;
  ChatOptions& opt_;
};

}  // namespace rendezllama
#endif


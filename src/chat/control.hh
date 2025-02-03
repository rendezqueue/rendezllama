#ifndef RENDEZLLAMA_CHAT_CONTROL_HH_
#define RENDEZLLAMA_CHAT_CONTROL_HH_
#include <string>

namespace rendezllama {

struct ChatOptions;
class ChatDisplay;
class ChatGuide;
class ChatTrajectory;
class Vocabulary;

class ChatControl {
 public:
  void increment_textgen_byte_count(unsigned n) {textgen_byte_count_ += n;}
  void reset_textgen_byte_limit(unsigned n) {textgen_byte_limit_ = n;}

  void accept_generated_token(
    ChatGuide& chat_guide,
    ChatDisplay& chat_disp,
    ChatTrajectory& chat_traj,
    std::string& matched_antiprompt,
    const ChatOptions& chat_opt,
    const Vocabulary& vocabulary);

  bool has_input_mode_on() const {return input_mode_on_;}
  void set_input_mode_on(bool b = true) {input_mode_on_ = b;}

  bool has_single_line_mode_on() const {return single_line_mode_on_;}
  void set_single_line_mode_on(bool b = true) {single_line_mode_on_ = b;}

 private:
  bool input_mode_on_ = true;
  bool single_line_mode_on_ = false;
  unsigned textgen_byte_count_ = 0;
  unsigned textgen_byte_limit_ = 0;
  unsigned textgen_sentence_token_count_ = 0;
  unsigned textgen_sentence_count_ = 0;
};

}  // namespace rendezllama
#endif

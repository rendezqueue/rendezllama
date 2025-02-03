#include "src/chat/control.hh"

#include "src/chat/display.hh"
#include "src/chat/guide.hh"
#include "src/chat/trajectory.hh"
#include "src/chat/opt.hh"

using rendezllama::ChatControl;
using rendezllama::ChatDisplay;
using rendezllama::ChatGuide;
using rendezllama::ChatOptions;
using rendezllama::ChatTrajectory;
using rendezllama::Vocabulary;

void ChatControl::accept_generated_token(
    ChatGuide& chat_guide,
    ChatDisplay& chat_disp,
    ChatTrajectory& chat_traj,
    std::string& matched_antiprompt,
    const ChatOptions& chat_opt,
    const Vocabulary& vocabulary)
{
  if (textgen_byte_limit_ > 0 && textgen_byte_count_ >= textgen_byte_limit_) {
    input_mode_on_ = true;
    chat_guide.end_turn();
    if (matched_antiprompt != "\n") {
      chat_disp.show_new(chat_traj, vocabulary);
    }
  }
  else if (chat_guide.maybe_yield_turn()) {
    if (matched_antiprompt != "\n") {
      matched_antiprompt = "\n";
    }
    if (chat_traj.message_prefix_id_ == 0) {
      input_mode_on_ = true;
    }
    chat_disp.show_new(chat_traj, vocabulary);
    textgen_sentence_count_ = 0;
    textgen_sentence_token_count_ = 0;
  }
  else if (!matched_antiprompt.empty()) {
    if (textgen_sentence_count_ + 1 == chat_opt.sentence_limit) {
      // Reached the limit on number of sentences.
      input_mode_on_ = true;
    }
    else {
      textgen_sentence_count_ += 1;
      textgen_sentence_token_count_ = 0;
    }
  }
  else {
    if (textgen_sentence_token_count_ + 1 == chat_opt.sentence_token_limit) {
      // Reached the limit on number of tokens in a sentence.
      input_mode_on_ = true;
    }
    else {
      textgen_sentence_token_count_ += 1;
    }
  }

  chat_disp.maybe_remove_answer_prompt(chat_traj, input_mode_on_);

  if (input_mode_on_) {
    textgen_byte_count_ = 0;
    textgen_sentence_count_ = 0;
    textgen_sentence_token_count_ = 0;
  }
}

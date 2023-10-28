#include "display.hh"

#include <cassert>

#include <fildesh/string.hh>

using rendezllama::ChatDisplay;
using rendezllama::ChatTrajectory;
using rendezllama::Vocabulary;

ChatDisplay::~ChatDisplay() {
  close_FildeshO(out_);
}

  void
ChatDisplay::displaystring_to(
    FildeshO* out,
    ChatTrajectory::Token_id token_id,
    const Vocabulary& vocabulary) const
{
  if (token_id == vocabulary.eos_token_id()) {
    // TODO(#30): remove hack.
    *out << "\n";
    //out = "";
  }
  else if (token_id == vocabulary.newline_token_id()) {
    // TODO(#30): remove hack.
    *out << " / ";
  }
  else {
    vocabulary.detokenize_to(out, token_id);
  }
}

  void
ChatDisplay::show_new(
    ChatTrajectory::size_type end,
    ChatTrajectory& chat_traj,
    const Vocabulary& vocabulary)
{
  assert(end <= chat_traj.token_count());
  while (chat_traj.display_token_count_ < end) {
    const ChatTrajectory::size_type i = chat_traj.display_token_count_;
    chat_traj.display_token_count_ += 1;
    if (answer_prompt_offset_ > 0 &&
        i >= answer_prompt_offset_ &&
        i < answer_prompt_offset_ + answer_prompt_tokens_.size())
    {
      continue;
    }
    this->displaystring_to(out_, chat_traj.token_at(i), vocabulary);
  }
  flush_FildeshO(out_);
}

  void
ChatDisplay::show_new(
    ChatTrajectory& chat_traj,
    const Vocabulary& vocabulary)
{
  this->show_new(chat_traj.token_count(), chat_traj, vocabulary);
  assert(chat_traj.display_token_count_ == chat_traj.token_count());
}

  void
ChatDisplay::maybe_insert_answer_prompt(
    ChatTrajectory& chat_traj,
    const Vocabulary& vocabulary)
{
  if (answer_prompt_tokens_.size() == 0) {
    assert(answer_prompt_offset_ == 0);
    return;
  }
  if (answer_prompt_offset_ != 0) {return;}
  answer_prompt_offset_ = chat_traj.token_count();
  while (answer_prompt_offset_ > 0) {
    if (vocabulary.last_char_of(chat_traj.token_at(answer_prompt_offset_-1)) == '\n') {
      break;
    }
    answer_prompt_offset_ -= 1;
  }
  if (answer_prompt_offset_ > 0) {
    chat_traj.insert_all_at(answer_prompt_offset_, answer_prompt_tokens_);
  }
}

  void
ChatDisplay::maybe_remove_answer_prompt(ChatTrajectory& chat_traj, bool inputting)
{
  if (!inputting) {return;}
  if (answer_prompt_tokens_.size() == 0) {return;}
  if (answer_prompt_offset_ == 0) {return;}
  chat_traj.erase_range(
      answer_prompt_offset_,
      answer_prompt_offset_ + answer_prompt_tokens_.size());
  answer_prompt_offset_ = 0;
}

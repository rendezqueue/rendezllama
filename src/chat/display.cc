#include "display.hh"

#include <cassert>

#include <fildesh/fildesh.h>

#include "llama.h"

#include "src/tokenize/tokenize.hh"

using rendezllama::ChatDisplay;
using rendezllama::ChatTrajectory;


ChatDisplay::~ChatDisplay() {
  close_FildeshO(out_);
}

  void
ChatDisplay::show_new(
    size_type end,
    ChatTrajectory& chat_traj,
    const struct llama_context* ctx)
{
  assert(end <= chat_traj.token_count());
  while (chat_traj.display_token_count_ < end) {
    const size_type i = chat_traj.display_token_count_;
    chat_traj.display_token_count_ += 1;
    if (answer_prompt_offset_ > 0 &&
        i >= answer_prompt_offset_ &&
        i < answer_prompt_offset_ + answer_prompt_tokens_.size())
    {
      continue;
    }
    puts_FildeshO(out_, llama_token_to_str(ctx, chat_traj.token_at(i)));
  }
  flush_FildeshO(out_);
}

  void
ChatDisplay::show_new(
    ChatTrajectory& chat_traj,
    const struct llama_context* ctx)
{
  this->show_new(chat_traj.token_count(), chat_traj, ctx);
  assert(chat_traj.display_token_count_ == chat_traj.token_count());
}

  void
ChatDisplay::maybe_insert_answer_prompt(
    ChatTrajectory& chat_traj,
    struct llama_context* ctx)
{
  if (answer_prompt_tokens_.size() == 0) {
    assert(answer_prompt_offset_ == 0);
    return;
  }
  if (answer_prompt_offset_ != 0) {return;}
  answer_prompt_offset_ = chat_traj.token_count();
  while (answer_prompt_offset_ > 0) {
    if (rendezllama::token_endswith(ctx, chat_traj.token_at(answer_prompt_offset_-1), '\n')) {
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

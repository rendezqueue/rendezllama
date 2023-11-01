#include "guide.hh"

#include <cassert>

#include <fildesh/fildesh.h>

#include "src/chat/opt.hh"
#include "src/chat/trajectory.hh"

using rendezllama::ChatGuide;
using rendezllama::ChatOptions;
using rendezllama::ChatTrajectory;
using rendezllama::Vocabulary;

static
  ChatTrajectory::message_prefix_id
next_turn_index(ChatTrajectory::message_prefix_id i, size_t n)
{
  assert(n != 0);
  return (i >= n-1 ? 0 : i+1);
}

  bool
ChatGuide::maybe_erase_trailing_message_prefix()
{
  if (traj_.priming_token_count() == traj_.token_count()) {
    // Pretend we deleted when the rolling prompt is empty.
    // This result is used to check whether a message suffix should be added,
    // which should only happen if a preceding message exists.
    return true;
  }
  auto i = traj_.token_count()-1;
  if (i != traj_.rfind_message_prefix_at(i)) {
    return false;
  }
  traj_.erase_all_at(traj_.rfind_message_prefix_begin_at(i));
  return true;
}

  bool
ChatGuide::maybe_erase_trailing_message_suffix()
{
  std::string_view suffix;  // Empty is treated as newline.
  const auto n = traj_.token_count();
  traj_.trim_message_suffix(suffix, vocab_);
  return (n != traj_.token_count());
}

  void
ChatGuide::begin_turn(unsigned turn_index)
{
  this->maybe_erase_trailing_message_prefix();
  std::string_view prefix = opt_.chat_prefixes[turn_index];
  traj_.tokenize_append_message_prefix(turn_index, prefix, vocab_);
}

  void
ChatGuide::end_turn()
{
  std::string_view suffix;  // Empty is treated as newline.
  traj_.tokenize_append_message_suffix(suffix, vocab_);
}

  void
ChatGuide::yield_turn(unsigned turn_index)
{
  if (!this->maybe_erase_trailing_message_prefix()) {
    this->end_turn();
  }
  this->begin_turn(turn_index);
}

  void
ChatGuide::yield_turn(std::string_view prefix)
{
  if (!this->maybe_erase_trailing_message_prefix()) {
    this->end_turn();
  }
  auto turn_index = traj_.unknown_message_prefix_id();
  for (size_t i = 0; i < opt_.chat_prefixes.size(); ++i) {
    std::string_view s = opt_.chat_prefixes[i];
    if (prefix.substr(0, s.size()) == s) {
      turn_index = i;
      break;
    }
  }
  traj_.tokenize_append_message_prefix(turn_index, prefix, vocab_);
}

  void
ChatGuide::yield_turn()
{
  this->yield_turn(
      next_turn_index(traj_.message_prefix_id_, opt_.chat_prefixes.size()));
}

  bool
ChatGuide::maybe_yield_turn()
{
  auto turn_index = traj_.message_prefix_id_;
  Vocabulary::Token_id token_id = traj_.token();
  if (token_id == vocab_.eos_token_id()) {
    // True.
  }
  else if (turn_index >= opt_.chat_prefixes.size()) {
    if (token_id != vocab_.newline_token_id()) {
      return false;
    }
  }
  else {
    if (opt_.multiline_confidant_on) {
      if (turn_index == opt_.chat_prefixes.size()-1) {
        return false;  // Needed EOS token.
      }
    }
    std::string_view suffix = "\n";
    if (!traj_.endswith_nonempty(suffix, vocab_)) {
      return false;
    }
  }
  this->yield_turn();
  return true;
}


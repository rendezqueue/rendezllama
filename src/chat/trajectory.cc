#include "trajectory.hh"

#include <cassert>
#include <climits>

#include <fildesh/fildesh.h>

#include "llama.h"

using rendezllama::ChatTrajectory;
using rendezllama::Vocabulary;

ChatTrajectory::ChatTrajectory(Token_id token_id) {
  token_values_.push_back(token_id);
  mirostat_mu_values_.push_back(0);
  message_prefix_ids_.push_back(this->not_a_message_prefix_id());
}

ChatTrajectory::~ChatTrajectory()
{
  close_FildeshO(this->transcript_out_);
}

  void
ChatTrajectory::push_back(Token_id token_id)
{
  token_values_.push_back(token_id);
  mirostat_mu_values_.push_back(this->mirostat_mu());
  message_prefix_ids_.push_back(this->not_a_message_prefix_id());
}

  void
ChatTrajectory::insert_all_at(
    size_type i, const std::vector<Token_id>& a)
{
  assert(i > 0);
  token_values_.insert(token_values_.begin() + i, a.begin(), a.end());
  mirostat_mu_values_.insert(
      mirostat_mu_values_.begin() + i,
      a.size(), this->mirostat_mu_at(i-1));
  message_prefix_ids_.insert(
      message_prefix_ids_.begin() + i,
      a.size(), this->not_a_message_prefix_id());
  if (i < display_token_count_) {
    display_token_count_ += a.size();
  }
}

  void
ChatTrajectory::erase_range(size_type beg, size_type end)
{
  erased_since_eval_ = true;
  assert(beg <= end);
  token_values_.erase(
      token_values_.begin() + beg,
      token_values_.begin() + end);
  if (context_token_count_ > beg) {
    context_token_count_ = beg;
  }
  if (context_token_count_ >= token_count()) {
    // The -1 is added to force an eval.
    context_token_count_ = token_count()-1;
  }
  mirostat_mu_values_.erase(
      mirostat_mu_values_.begin() + beg,
      mirostat_mu_values_.begin() + end);
  message_prefix_ids_.erase(
      message_prefix_ids_.begin() + beg,
      message_prefix_ids_.begin() + end);
  if (beg < display_token_count_) {
    if (end < display_token_count_) {
      display_token_count_ -= (end - beg);
    }
    else {
      display_token_count_ = beg;
    }
  }
  message_prefix_id_ = last_message_prefix_id_at(this->token_count());
}

  void
ChatTrajectory::rollforget(size_type end, const Vocabulary& vocabulary)
{
  assert(end <= this->token_count());
  const size_type beg = priming_token_count_;
  if (transcript_out_) {
    for (size_type i = beg; i < end; ++i) {
      vocabulary.detokenize_to(transcript_out_, this->token_at(i));
    }
    flush_FildeshO(transcript_out_);
  }
  this->erase_range(beg, end);
}

  void
ChatTrajectory::tokenize_append(
    std::string_view s,
    const Vocabulary& vocabulary)
{
  std::vector<llama_token> tmp;
  vocabulary.tokenize_to(tmp, s);
  this->insert_all_at(this->token_count(), tmp);
}

  void
ChatTrajectory::tokenize_append_message_prefix(
    unsigned id,
    std::string_view s,
    const Vocabulary& vocabulary)
{
  std::vector<llama_token> tmp;
  vocabulary.tokenize_to(tmp, s);
  size_t i = this->token_count();
  this->insert_all_at(this->token_count(), tmp);
  for (; i < this->token_count(); ++i) {
    message_prefix_ids_[i] = id;
  }
  message_prefix_id_ = id;
}

  ChatTrajectory::size_type
ChatTrajectory::rfind_message_prefix_at(size_type i) const
{
  while (i >= priming_token_count_) {
    if (message_prefix_ids_[i] != this->not_a_message_prefix_id()) {
      break;
    }
    i -= 1;
  }
  return i;
}

  ChatTrajectory::size_type
ChatTrajectory::rfind_message_prefix_begin_at(size_type i) const
{
  i = this->rfind_message_prefix_at(i);
  while (i > priming_token_count_) {
    if (message_prefix_ids_[i-1] != message_prefix_ids_[i]) {
      break;
    }
    i -= 1;
  }
  return i;
}

  ChatTrajectory::size_type
ChatTrajectory::rfind_last_message_prefix_end_at(size_type i) const
{
  size_type e;
  if (i < this->token_count()) {
    e = rfind_message_prefix_at(i);
  }
  else {
    e = rfind_message_prefix_at(this->token_count()-1);
  }

  if (e < i) {
    i = e;
  }
  else {
    i = rfind_message_prefix_begin_at(i);
    if (i > priming_token_count_) {
      i = rfind_message_prefix_at(i-1);
    }
  }
  if (message_prefix_ids_[i] == this->not_a_message_prefix_id()) {
    return i;
  }
  return i + 1;
}

  ChatTrajectory::size_type
ChatTrajectory::last_message_prefix_id_at(size_type i) const
{
  i = rfind_last_message_prefix_end_at(i);
  return message_prefix_ids_[i-1];
}

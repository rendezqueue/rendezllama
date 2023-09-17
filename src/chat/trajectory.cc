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
  line_prefix_indices_.push_back(UINT_MAX);
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
  line_prefix_indices_.push_back(this->line_prefix_index());
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
  line_prefix_indices_.insert(
      line_prefix_indices_.begin() + i,
      a.size(), this->line_prefix_index_at(i-1));
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
  line_prefix_indices_.erase(
      line_prefix_indices_.begin() + beg,
      line_prefix_indices_.begin() + end);
  if (beg < display_token_count_) {
    if (end < display_token_count_) {
      display_token_count_ -= (end - beg);
    }
    else {
      display_token_count_ = beg;
    }
  }
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


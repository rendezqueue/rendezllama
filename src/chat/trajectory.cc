#include "trajectory.hh"

#include <cassert>

#include <fildesh/fildesh.h>

#include "llama.h"

using rendezllama::ChatTrajectory;

ChatTrajectory::ChatTrajectory() {
  this->push_back(llama_token_bos());
}

ChatTrajectory::~ChatTrajectory()
{
  close_FildeshO(this->transcript_out_);
}

  void
ChatTrajectory::push_back(Token_id token_id)
{
  token_values_.push_back(token_id);
}

  void
ChatTrajectory::insert_all_at(
    size_type i, const std::vector<Token_id>& a)
{
  token_values_.insert(token_values_.begin() + i, a.begin(), a.end());
}

  void
ChatTrajectory::erase_range(size_type beg, size_type end)
{
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
}

  void
ChatTrajectory::rollforget(size_type end, const struct llama_context* ctx)
{
  assert(end <= this->token_count());
  const size_type beg = priming_token_count_;
  if (transcript_out_) {
    for (size_type i = beg; i < end; ++i) {
      puts_FildeshO(transcript_out_, llama_token_to_str(ctx, this->token_at(i)));
    }
    flush_FildeshO(transcript_out_);
  }
  this->erase_range(beg, end);
}


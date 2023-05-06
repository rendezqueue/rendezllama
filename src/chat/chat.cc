#include "chat.hh"

#include <algorithm>
#include <cassert>
#include <cstring>

#include <fildesh/fildesh.h>

#include "src/chat/display.hh"
#include "src/chat/opt.hh"
#include "src/chat/trajectory.hh"
#include "src/tokenize/tokenize.hh"

using rendezllama::ChatDisplay;
using rendezllama::ChatTrajectory;


  const std::string&
rendezllama::antiprompt_suffix(
    const std::string& text,
    const std::vector<std::string>& antiprompts)
{
  static const std::string empty_string;
  for (const std::string& s : antiprompts) {
    if (text.size() >= s.size()) {
      const size_t offset = text.size() - s.size();
      if (0 == memcmp(&text[offset], &s[0], s.size())) {
        return s;
      }
    }
  }
  return empty_string;
}

static std::string protagonist_line_prefix(const rendezllama::ChatOptions& opt) {
  std::string s;
  if (opt.linespace_on) {s += ' ';}
  s += opt.protagonist + ": ";
  return s;
}
static std::string confidant_line_prefix(const rendezllama::ChatOptions& opt) {
  std::string s;
  if (opt.linespace_on) {s += ' ';}
  s += opt.confidant + ':';
  return s;
}

static bool maybe_trim_endspace(std::string& s)
{
  bool result = false;
  while (!s.empty() && s.back() == ' ') {
    s.pop_back();
    result = true;
  }
  return result;
}

  void
rendezllama::augment_chat_input(
    std::string& s,
    bool& prevent_subsequent_newline,
    const std::string& matched_antiprompt,
    const rendezllama::ChatOptions& opt)
{
  prevent_subsequent_newline = false;
  if (s.size() >= 2 && s[0] == '\\' && s[1] == 'n') {
    s.erase(0, 2);
    std::string maybe_newline, maybe_space;
    if (matched_antiprompt != "\n") {
      maybe_newline = '\n';
    }
    if (s.empty() || s[0] != ' ') {
      maybe_space = ' ';
    }
    s = maybe_newline + confidant_line_prefix(opt) + maybe_space + s;
    prevent_subsequent_newline = maybe_trim_endspace(s);
  }
  else if (s.front() == '\n') {
    s.erase(0, 1);
    std::string pfx;
    // This is from /yield.
    if (matched_antiprompt != "\n") {
      pfx += '\n';
    }
    if (opt.linespace_on) {pfx += ' ';}
    s = pfx + s;
  }
  else if (s.front() == ' ') {
    prevent_subsequent_newline = maybe_trim_endspace(s);
  }
  else if (s.front() == '\n') {
    // This is from /yield.
    s.erase(0, 1);
    std::string pfx;
    if (matched_antiprompt != "\n") {
      pfx += '\n';
    }
    if (opt.linespace_on) {pfx += ' ';}
    s = pfx + s;
  }
  else if (s.back() == '[' || s.back() == ':') {
    // Nothing.
  }
  else {
    std::string maybe_newline;
    if (matched_antiprompt != "\n") {
      maybe_newline = '\n';
    }
    s = (maybe_newline + protagonist_line_prefix(opt) +
         s + '\n' + confidant_line_prefix(opt));
  }
}

  void
rendezllama::tokenize_extend(
    ChatTrajectory& chat_traj,
    llama_context* ctx, const std::string& s)
{
  std::vector<llama_token> tokens;
  rendezllama::tokenize_extend(tokens, ctx, s);
  chat_traj.insert_all_at(chat_traj.token_count(), tokens);
}

static
  void
temperature_based_sample(
    llama_token_data_array* candidates_data,
    ChatTrajectory& chat_traj,
    struct llama_context* ctx,
    const rendezllama::ChatOptions& opt)
{
  const unsigned keep_one = 1;
  llama_sample_top_k(ctx, candidates_data, opt.top_k, keep_one);
  llama_sample_tail_free(ctx, candidates_data, opt.tfs_z, keep_one);
  llama_sample_typical(ctx, candidates_data, opt.typical_p, keep_one);
  llama_sample_top_p(ctx, candidates_data, opt.top_p, keep_one);
  llama_sample_temperature(ctx, candidates_data, opt.temp);
  chat_traj.push_back(llama_sample_token(ctx, candidates_data));
}

static
  void
mirostat1_sample(
    llama_token_data_array* candidates_data,
    ChatTrajectory& chat_traj,
    struct llama_context* ctx,
    const rendezllama::ChatOptions& opt)
{
  float mirostat_mu = chat_traj.mirostat_mu();
  const int mirostat_m = 100;
  llama_sample_temperature(ctx, candidates_data, opt.temp);
  chat_traj.push_back(llama_sample_token_mirostat(
      ctx, candidates_data, opt.mirostat_tau, opt.mirostat_eta, mirostat_m, &mirostat_mu));
  chat_traj.mirostat_mu() = mirostat_mu;
}

static
  void
mirostat2_sample(
    llama_token_data_array* candidates_data,
    ChatTrajectory& chat_traj,
    struct llama_context* ctx,
    const rendezllama::ChatOptions& opt)
{
  float mirostat_mu = chat_traj.mirostat_mu();
  llama_sample_temperature(ctx, candidates_data, opt.temp);
  chat_traj.push_back(llama_sample_token_mirostat_v2(
      ctx, candidates_data, opt.mirostat_tau, opt.mirostat_eta, &mirostat_mu));
  chat_traj.mirostat_mu() = mirostat_mu;
}

  void
rendezllama::generate_next_token(
    ChatTrajectory& chat_traj,
    struct llama_context* ctx,
    bool preventing_newline,
    const std::vector<llama_token>& extra_penalized_tokens,
    const rendezllama::ChatOptions& opt)
{
  float* logits = llama_get_logits(ctx);
  if (preventing_newline) {
    // Zero probability for message-ending tokens when requested.
    logits[llama_token_eos()] = 0;
    logits[rendezllama::newline_token(ctx)] = 0;
  }

  const size_t trailing_token_count = std::min(
      chat_traj.token_count(), opt.repeat_last_count);

  std::vector<llama_token> penalized_tokens;
  penalized_tokens.resize(trailing_token_count);
  for (unsigned i = 0; i < trailing_token_count; ++i) {
    penalized_tokens[i] = chat_traj.token_at(
        chat_traj.token_count() - trailing_token_count + i);
  }
  penalized_tokens.insert(
      penalized_tokens.end(),
      extra_penalized_tokens.begin(), extra_penalized_tokens.end());

  std::vector<llama_token_data> candidates;
  candidates.resize(llama_n_vocab(ctx));
  for (llama_token i = 0; i < (llama_token)candidates.size(); ++i) {
    candidates[i] = llama_token_data{
      i, logits[i], 0.0f,
    };
  }
  llama_token_data_array candidates_data[1] = {{
    candidates.data(), candidates.size(), false,
  }};

  llama_sample_repetition_penalty(
      ctx, candidates_data,
      penalized_tokens.data(), penalized_tokens.size(),
      opt.repeat_penalty);
  llama_sample_frequency_and_presence_penalties(
      ctx, candidates_data,
      penalized_tokens.data(), penalized_tokens.size(),
      opt.frequency_penalty, opt.presence_penalty);

  if (opt.mirostat_sampling == 1) {
    mirostat1_sample(candidates_data, chat_traj, ctx, opt);
  }
  else if (opt.mirostat_sampling == 2) {
    mirostat2_sample(candidates_data, chat_traj, ctx, opt);
  }
  else {
    temperature_based_sample(candidates_data, chat_traj, ctx, opt);
  }

  // Interpret end-of-stream (technically "end-of-sentence" as a newline token.
  if (chat_traj.token() == llama_token_eos()) {
    llama_token token_id = llama_token_eos();
    int n = llama_tokenize(ctx, "\n", &token_id, 1, /*add_bos=*/false);
    assert(n == 1);
    chat_traj.push_back(token_id);
    chat_traj.erase_range(chat_traj.token_count()-2, chat_traj.token_count()-1);
  }
}

  bool
rendezllama::commit_to_context(
    struct llama_context* ctx,
    ChatDisplay& chat_disp,
    ChatTrajectory& chat_traj,
    const ChatOptions& opt)
{
  assert(chat_traj.context_token_count_ < chat_traj.token_count());

  if (chat_traj.token_count() > opt.context_token_limit) {
    // Drop some of the rolling prompt while keeping the priming prompt
    // to avoid exceeding our context token limit.
    const unsigned rolling_token_count =
      (opt.context_token_limit - chat_traj.priming_token_count_) / 2;
    for (unsigned i = chat_traj.token_count() - rolling_token_count;
         i < chat_traj.token_count();
         ++i)
    {
      if (rendezllama::token_endswith(ctx, chat_traj.token_at(i), '\n')) {
        chat_traj.rollforget(i+1, ctx);
        break;
      }
    }
    if (chat_traj.token_count() >= opt.context_token_limit) {
      chat_traj.rollforget(chat_traj.token_count() - rolling_token_count, ctx);
    }
  }

  while (chat_traj.context_token_count_ < chat_traj.token_count()) {
    const unsigned n = std::min(
        opt.batch_count,
        chat_traj.token_count() - chat_traj.context_token_count_);

    chat_disp.show_new(chat_traj.context_token_count_ + n, chat_traj, ctx);

    const int istat = llama_eval(
        ctx,
        &chat_traj.token_at(chat_traj.context_token_count_),
        n,
        chat_traj.context_token_count_,
        opt.thread_count);
    if (istat != 0) {
      fildesh_log_error("Failed to eval.");
      chat_traj.context_token_count_ = 0;
      return false;
    }
    else {
      chat_traj.context_token_count_ += n;
    }
  }
  assert(chat_traj.context_token_count_ == chat_traj.token_count());
  return true;
}


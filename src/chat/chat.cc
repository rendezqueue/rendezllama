#include "chat.hh"

#include <algorithm>
#include <cassert>
#include <cstring>

#include <fildesh/fildesh.h>

#include "src/chat/display.hh"
#include "src/chat/opt.hh"
#include "src/chat/trajectory.hh"
#include "src/language/vocabulary.hh"

using rendezllama::ChatDisplay;
using rendezllama::ChatOptions;
using rendezllama::ChatTrajectory;
using rendezllama::Vocabulary;


  const std::string&
rendezllama::antiprompt_suffix(
    std::string_view text,
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

static bool maybe_trim_endspace(std::string& s)
{
  bool result = false;
  while (!s.empty() && s.back() == ' ') {
    s.pop_back();
    result = true;
  }
  return result;
}

static
  bool
eom_newline_check(
    const ChatOptions& opt,
    const ChatTrajectory& chat_traj)
{
  if (chat_traj.message_prefix_id_ < opt.chat_prefixes.size()-1) {
    return true;
  }
  return !opt.multiline_confidant_on;
}

  bool
rendezllama::eom_token_check(
    const Vocabulary& vocabulary,
    llama_token token_id,
    const ChatOptions& opt,
    const ChatTrajectory& chat_traj)

{
  if (token_id == vocabulary.eos_token_id()) {
    return true;
  }
  if (eom_newline_check(opt, chat_traj)) {
    return token_id == vocabulary.newline_token_id();
  }
  return false;
}

  void
rendezllama::augment_tokenize_chat_input(
    ChatTrajectory& chat_traj,
    bool& prevent_subsequent_newline,
    std::string s,
    const std::string& matched_antiprompt,
    const Vocabulary& vocabulary,
    const ChatOptions& opt)
{
  const char* const maybe_space_prefix = (
      opt.chat_prefixes[0].back() == '\n'
      ? "" : " ");
  prevent_subsequent_newline = false;
  if (s.size() >= 2 && s[0] == '\\' && s[1] == 'n') {
    s.erase(0, 2);
    std::string maybe_space;
    if (matched_antiprompt != "\n") {
      chat_traj.push_back(vocabulary.newline_token_id());
    }
    if (s.empty() || s[0] != ' ') {
      maybe_space = ' ';
    }
    chat_traj.tokenize_append_message_prefix(
        opt.chat_prefixes.size()-1,
        opt.chat_prefixes.back(),
        vocabulary);

    prevent_subsequent_newline = maybe_trim_endspace(s);
    chat_traj.tokenize_append(
        maybe_space + s,
        vocabulary);
  }
  else if (s.front() == '\n') {
    s.erase(0, 1);
    // This is from /yield.
    if (matched_antiprompt != "\n") {
      chat_traj.push_back(vocabulary.newline_token_id());
    }
    if (opt.linespace_on) {s = ' ' + s;}
    chat_traj.tokenize_append_message_prefix(
        chat_traj.unknown_message_prefix_id(),
        s,
        vocabulary);
  }
  else if (s.front() == ' ') {
    prevent_subsequent_newline = maybe_trim_endspace(s);
    chat_traj.tokenize_append(s, vocabulary);
  }
  else if (s.back() == '[' || s.back() == ':') {
    chat_traj.tokenize_append(s, vocabulary);
  }
  else if (matched_antiprompt == opt.chat_prefixes[0]) {
    chat_traj.tokenize_append(maybe_space_prefix + s + '\n', vocabulary);
    chat_traj.display_token_count_ = chat_traj.token_count();
    chat_traj.tokenize_append_message_prefix(
        1,
        opt.chat_prefixes[1],
        vocabulary);
    prevent_subsequent_newline = true;
  }
  else {
    if (matched_antiprompt != "\n") {
      chat_traj.push_back(vocabulary.newline_token_id());
    }
    chat_traj.tokenize_append_message_prefix(
        0,
        opt.chat_prefixes[0],
        vocabulary);
    chat_traj.tokenize_append(
        maybe_space_prefix + s + '\n',
        vocabulary);
    chat_traj.tokenize_append_message_prefix(
        1,
        opt.chat_prefixes[1],
        vocabulary);
    prevent_subsequent_newline = true;
  }
}

  std::tuple<struct llama_model*, struct llama_context*>
rendezllama::make_llama_context(rendezllama::ChatOptions& opt)
{
  llama_model_params model_params = llama_model_default_params();
  model_params.vocab_only = true;

  struct llama_model* model = llama_load_model_from_file(
      opt.model_filename.c_str(), model_params);
  if (!model) {
    fildesh_log_error("Failed to open model.");
    return std::make_tuple(nullptr, nullptr);
  }

  if (opt.model_token_limit == 0) {
    opt.model_token_limit = llama_n_ctx_train(model);
  }
  if (opt.context_token_limit == 0) {
    opt.context_token_limit = opt.model_token_limit;
  }

  model_params = llama_model_default_params();
  model_params.use_mlock = opt.mlock_on;
  model_params.use_mmap = opt.mmap_on;

  llama_context_params ctx_params = llama_context_default_params();
  ctx_params.n_ctx = opt.context_token_limit;
  ctx_params.seed = opt.seed;
  ctx_params.f16_kv = true;
  ctx_params.rope_freq_scale = llama_rope_freq_scale_train(model);
  ctx_params.n_threads = opt.thread_count;
  ctx_params.n_batch = opt.batch_count;
  while (
      (unsigned)(opt.model_token_limit / ctx_params.rope_freq_scale)
      <
      opt.context_token_limit)
  {
    ctx_params.rope_freq_scale /= 2;
  }

  llama_free_model(model);
  model = llama_load_model_from_file(
      opt.model_filename.c_str(), model_params);
  if (!model) {
    fildesh_log_error("Failed to open model.");
    return std::make_tuple(nullptr, nullptr);
  }

  struct llama_context* ctx = llama_new_context_with_model(model, ctx_params);
  if (!ctx) {
    llama_free_model(model);
    fildesh_log_error("Failed to create context.");
    return std::make_tuple(nullptr, nullptr);
  }
  return std::make_tuple(model, ctx);
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
  llama_sample_temp(ctx, candidates_data, opt.temperature);
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
  llama_sample_temp(ctx, candidates_data, opt.temperature);
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
  llama_sample_temp(ctx, candidates_data, opt.temperature);
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
  const Vocabulary vocabulary(ctx);
  float* logits = llama_get_logits(ctx);
  if (preventing_newline) {
    // Zero probability for message-ending tokens when requested.
    logits[vocabulary.eos_token_id()] = 0;
    logits[vocabulary.newline_token_id()] = 0;
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
  candidates.resize(vocabulary.cardinality());
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
  if (chat_traj.token() == vocabulary.eos_token_id() && eom_newline_check(opt, chat_traj)) {
    chat_traj.push_back(vocabulary.newline_token_id());
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
  const Vocabulary vocabulary(ctx);
  assert(!chat_traj.erased_since_eval_ ||
         chat_traj.context_token_count_ < chat_traj.token_count());
  if (chat_traj.context_token_count_ == chat_traj.token_count()) {
    return true;
  }

  chat_traj.maybe_rollforget_within_limit(opt.context_token_limit, vocabulary);

  // Reset thread count just in case the user reconfigured it.
  llama_set_n_threads(ctx, opt.thread_count, opt.thread_count);
  // Clear KV cache past current position just in case the user deleted tokens.
  llama_kv_cache_tokens_rm(ctx, chat_traj.context_token_count_, -1);

  while (chat_traj.context_token_count_ < chat_traj.token_count()) {
    const unsigned n = std::min(
        opt.batch_count,
        chat_traj.token_count() - chat_traj.context_token_count_);

    chat_disp.show_new(chat_traj.context_token_count_ + n, chat_traj, vocabulary);

    llama_batch batch = llama_batch_get_one(
        const_cast<int*>(&chat_traj.tokens()[chat_traj.context_token_count_]),
        n,
        chat_traj.context_token_count_,
        0);
    const int istat = llama_decode(ctx, batch);
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
  chat_traj.erased_since_eval_ = false;
  return true;
}


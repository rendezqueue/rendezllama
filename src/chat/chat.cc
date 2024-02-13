#include "src/language/inference.hh"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <thread>

#include <fildesh/fildesh.h>
#include <fildesh/string.hh>

#include "src/chat/display.hh"
#include "src/chat/guide.hh"
#include "src/chat/opt.hh"
#include "src/chat/trajectory.hh"
#include "src/language/vocabulary.hh"

using rendezllama::ChatDisplay;
using rendezllama::ChatGuide;
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

  void
rendezllama::augment_tokenize_chat_input(
    ChatGuide& chat_guide,
    ChatTrajectory& chat_traj,
    bool& prevent_subsequent_newline,
    std::string s,
    const Vocabulary& vocabulary,
    const ChatOptions& opt)
{
  prevent_subsequent_newline = false;
  if (s.size() >= 2 && s[0] == '\\' && s[1] == 'n') {
    chat_guide.end_turn();
    chat_guide.begin_turn(opt.message_opts.size()-1);
    s.erase(0, 2);
    prevent_subsequent_newline = maybe_trim_endspace(s);
    if (opt.message_opts.back().prefix.back() != '\n' || opt.linespace_on) {
      if (!s.empty() && s.front() != ' ') {
        s.insert(0, " ");
      }
    }
    chat_traj.tokenize_append(s, vocabulary);
  }
  else if (s.front() == '\n') {
    // This is from /yield.
    chat_guide.yield_turn(s.substr(1));
  }
  else if (s.front() == ' ') {
    prevent_subsequent_newline = maybe_trim_endspace(s);
    chat_traj.tokenize_append(s, vocabulary);
  }
  else {
    chat_guide.yield_turn(0);
    if (opt.message_opts[0].prefix.back() != '\n' || opt.linespace_on) {
      if (!s.empty() && s.front() != ' ') {
        s.insert(0, " ");
      }
    }
    chat_traj.tokenize_append(s, vocabulary);
    chat_guide.yield_turn();
    chat_traj.display_token_count_ = chat_traj.rfind_message_prefix_begin_at(
        chat_traj.token_count()-1);
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
  llama_sample_min_p(ctx, candidates_data, opt.min_p, keep_one);
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
    const Vocabulary& vocabulary,
    const ChatOptions& opt)
{
  float* logits = llama_get_logits(ctx);
  if (preventing_newline) {
    // Zero probability for message-ending tokens when requested.
    logits[vocabulary.eos_token_id()] = 0;
    logits[vocabulary.newline_token_id()] = 0;
  }

  const size_t trailing_token_count = std::min(
      chat_traj.token_count(), opt.repeat_last_count);

  std::vector<llama_token> penalized_tokens;
  penalized_tokens.reserve(trailing_token_count);
  fildesh::ostringstream oss;
  for (unsigned i = 0; i < trailing_token_count; ++i) {
    Vocabulary::Token_id token_id = chat_traj.token_at(
        chat_traj.token_count() - trailing_token_count + i);
    oss.truncate();
    vocabulary.detokenize_to(oss, token_id);
    const std::string& matched = rendezllama::antiprompt_suffix(
        oss.view(), opt.antiprompts);
    if (matched.empty()) {
      penalized_tokens.push_back(token_id);
    }
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

  llama_sample_repetition_penalties(
      ctx, candidates_data,
      penalized_tokens.data(), penalized_tokens.size(),
      opt.repeat_penalty,
      opt.frequency_penalty,
      opt.presence_penalty);

  if (opt.mirostat_sampling == 1) {
    mirostat1_sample(candidates_data, chat_traj, ctx, opt);
  }
  else if (opt.mirostat_sampling == 2) {
    mirostat2_sample(candidates_data, chat_traj, ctx, opt);
  }
  else {
    temperature_based_sample(candidates_data, chat_traj, ctx, opt);
  }
}

  bool
rendezllama::commit_to_context(
    struct llama_context* ctx,
    ChatDisplay& chat_disp,
    ChatTrajectory& chat_traj,
    const Vocabulary& vocabulary,
    const ChatOptions& opt)
{
  assert(!chat_traj.erased_since_eval_ ||
         chat_traj.context_token_count_ < chat_traj.token_count());
  if (chat_traj.context_token_count_ == chat_traj.token_count()) {
    return true;
  }

  chat_traj.maybe_rollforget_within_limit(opt.context_token_limit, vocabulary);

  // Reset thread count just in case the user reconfigured it.
  const unsigned thread_count = opt.thread_count;
  unsigned batch_thread_count = opt.batch_thread_count;
  if (batch_thread_count == 0) {
    batch_thread_count = std::thread::hardware_concurrency();
  }
  if (batch_thread_count == 0) {
    batch_thread_count = thread_count;
  }
  llama_set_n_threads(ctx, thread_count, batch_thread_count);

  // Clear KV cache past current position just in case the user deleted tokens.
  llama_kv_cache_seq_rm(ctx, -1, chat_traj.context_token_count_, -1);

  while (chat_traj.context_token_count_ < chat_traj.token_count()) {
    const unsigned n = std::min(
        opt.batch_count,
        chat_traj.token_count() - chat_traj.context_token_count_);

#if LLAMA_OPENBLAS_ON
    if (n < 32) {
      llama_set_n_threads(ctx, thread_count, batch_thread_count);
    }
    else {
      llama_set_n_threads(ctx, thread_count, 1);
    }
#endif
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


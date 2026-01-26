#include "src/language/inference.hh"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <vector>

#include <fildesh/fildesh.h>
#include <fildesh/ostream.hh>

#include "src/chat/display.hh"
#include "src/chat/guide.hh"
#include "src/chat/opt.hh"
#include "src/chat/trajectory.hh"
#include "src/language/vocabulary.hh"

using rendezllama::ChatDisplay;
using rendezllama::ChatGuide;
using rendezllama::ChatOptions;
using rendezllama::ChatTrajectory;
using rendezllama::Inference;
using rendezllama::Vocabulary;
using rendezllama::inference::AdjustViaKind;

Inference::Inference(const Vocabulary& vocabulary)
  : vocabulary_(vocabulary)
{}
Inference::~Inference() {
  if (smpl_) {llama_sampler_free(smpl_);}
  llama_batch_free(batch_);
}

  const std::string&
rendezllama::antiprompt_suffix(
    std::string_view text,
    const std::set<std::string>& antiprompts)
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
    if (opt.message_opts.back().prefix.back() == '\n' && opt.linespace_on) {
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
    if (opt.message_opts[0].prefix.back() == '\n' && opt.linespace_on) {
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
  model_params.use_mlock = opt.mlock_on;
  model_params.use_mmap = opt.mmap_on;

  struct llama_model* model = llama_model_load_from_file(
      opt.model_filename.c_str(), model_params);
  if (!model) {
    fildesh_log_error("Failed to open model.");
    return std::make_tuple(nullptr, nullptr);
  }

  if (opt.model_token_limit == 0) {
    opt.model_token_limit = llama_model_n_ctx_train(model);
  }
  if (opt.context_token_limit == 0) {
    opt.context_token_limit = opt.model_token_limit;
  }
  float rope_freq_scale = llama_model_rope_freq_scale_train(model);
  if (rope_freq_scale <= 0.0) {
    rope_freq_scale = 1.0f;
  }
  while (
      (unsigned)(opt.model_token_limit / rope_freq_scale)
      <
      opt.context_token_limit)
  {
    rope_freq_scale /= 2;
  }
  llama_model_free(model);
  model = nullptr;


  model_params = llama_model_default_params();
  model_params.use_mlock = opt.mlock_on;
  model_params.use_mmap = opt.mmap_on;

  llama_context_params ctx_params = llama_context_default_params();
  ctx_params.n_ctx = opt.context_token_limit;
  ctx_params.n_batch = opt.batch_count;
  ctx_params.rope_freq_scale = rope_freq_scale;

  std::vector<float> tensor_split(llama_max_devices());
  std::vector<llama_model_tensor_buft_override> tensor_buft_overrides(llama_max_tensor_buft_overrides());
  std::vector<size_t> margins(llama_max_devices(), 0);

  // Auto-tune parameters if possible (and not manually overridden by user yet).
  // This helps avoid OOM crashes on Vulkan/GPU by fitting layers to available memory.
  auto status = llama_params_fit(
      opt.model_filename.c_str(),
      &model_params,
      &ctx_params,
      tensor_split.data(),
      tensor_buft_overrides.data(),
      margins.data(),
      /*n_ctx_min=*/0,
      GGML_LOG_LEVEL_ERROR);

  if (status != 0) {
    fildesh_log_warning("llama_params_fit failed");
  }

  model = llama_model_load_from_file(
      opt.model_filename.c_str(), model_params);
  if (!model) {
    fildesh_log_error("Failed to open model.");
    return std::make_tuple(nullptr, nullptr);
  }

  struct llama_context* ctx = llama_init_from_model(model, ctx_params);
  if (!ctx) {
    llama_model_free(model);
    fildesh_log_error("Failed to create context.");
    return std::make_tuple(nullptr, nullptr);
  }
  return std::make_tuple(model, ctx);
}

static
  int
new_sampling_seed()
{
  return static_cast<int>(INT_MAX & time(NULL));
}

static
  void
apply_sampler_chain(
    struct llama_sampler* smpl,
    const rendezllama::inference::AdjustVia& adjust_via,
    const struct llama_model* model,
    unsigned seed,
    std::ostream& eout)
{
  const unsigned keep_one = 1;

  if (const auto* dry = std::get_if<AdjustViaKind::dry>(&adjust_via)) {
    static const char* seq_breakers[] = {
      "\n", ":",
    };
    llama_sampler_init_dry(
        llama_model_get_vocab(model),
        llama_model_n_ctx_train(model),
        dry->multiplier,
        dry->base,
        dry->allowed_length,
        dry->window_length,
        seq_breakers,
        sizeof(seq_breakers)/sizeof(*seq_breakers));
    eout << "dry:"
      << "\n  multiplier: " << dry->multiplier
      << "\n  base: " << dry->base
      << "\n  allowed_length: " << dry->allowed_length
      << "\n  window_length: " << dry->window_length
      << "\n";
  }
  if (const auto* min_p = std::get_if<AdjustViaKind::min_p>(&adjust_via)) {
    llama_sampler_chain_add(smpl, llama_sampler_init_min_p(*min_p, keep_one));
    eout << "min_p: " << *min_p << "\n";
  }
  if (const auto* penalize_with = std::get_if<AdjustViaKind::penalize_with>(&adjust_via)) {
    llama_sampler_init_penalties(
        penalize_with->window_length,
        penalize_with->repetition,
        penalize_with->frequency,
        penalize_with->presence);
    eout << "penalties:"
      << "\n  window_length: " << penalize_with->window_length
      << "\n  repetition: " << penalize_with->repetition
      << "\n  frequency: " << penalize_with->frequency
      << "\n  presence: " << penalize_with->presence
      << "\n";
  }
  if (const auto* temperature = std::get_if<AdjustViaKind::temperature>(&adjust_via)) {
    llama_sampler_chain_add(smpl, llama_sampler_init_temp(*temperature));
    eout << "temperature: " << *temperature << "\n";
  }
  if (const auto* top_k = std::get_if<AdjustViaKind::top_k>(&adjust_via)) {
    llama_sampler_chain_add(smpl, llama_sampler_init_top_k(*top_k));
    eout << "top_k: " << *top_k << "\n";
  }
  if (const auto* top_p = std::get_if<AdjustViaKind::top_p>(&adjust_via)) {
    llama_sampler_chain_add(smpl, llama_sampler_init_top_p(*top_p, keep_one));
    eout << "top_p: " << *top_p << "\n";
  }
  if (const auto* typical_p = std::get_if<AdjustViaKind::typical_p>(&adjust_via)) {
    llama_sampler_chain_add(smpl, llama_sampler_init_typical(*typical_p, keep_one));
    eout << "typical_p: " << *typical_p << "\n";
  }
  if (const auto* xtc = std::get_if<AdjustViaKind::xtc>(&adjust_via)) {
    llama_sampler_chain_add(smpl, llama_sampler_init_xtc(xtc->probability, xtc->threshold, keep_one, seed));
    eout << "xtc: "
      << "\n  probability: " << xtc->probability
      << "\n  threshold: " << xtc->threshold
      << "\n";
  }
}

static
  void
adaptive_p_sample(
    struct llama_sampler* smpl,
    const rendezllama::inference::AdaptiveP& adaptive_p,
    unsigned seed)
{
  llama_sampler_chain_add(
      smpl,
      llama_sampler_init_adaptive_p(
          adaptive_p.target,
          adaptive_p.decay,
          seed));
}

static
  void
mirostat_sample(
    struct llama_sampler* smpl,
    const rendezllama::inference::Mirostat& mirostat,
    unsigned seed,
    const rendezllama::Vocabulary& vocabulary)
{
  if (mirostat.version == 1) {
    const int mirostat_m = 100;
    llama_sampler_chain_add(
        smpl,
        llama_sampler_init_mirostat(
            vocabulary.cardinality(), seed,
            mirostat.tau, mirostat.eta, mirostat_m));
  }
  else if (mirostat.version == 2) {
    llama_sampler_chain_add(
        smpl,
        llama_sampler_init_mirostat_v2(
            seed, mirostat.tau, mirostat.eta));
  }
}

static
  std::tuple<unsigned, unsigned>
infer_thread_counts(const rendezllama::ChatOptions& opt)
{
  unsigned thread_count = opt.thread_count;
  unsigned batch_thread_count = opt.batch_thread_count;
  const unsigned n = std::thread::hardware_concurrency();
  if (thread_count == 0) {
    thread_count = n / 2;
    if (thread_count == 0) {
      thread_count = 1;
    }
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    if (2 <= n && n <= 4) {
      thread_count = n;
    }
#endif
  }
  if (batch_thread_count == 0) {
    batch_thread_count = n;
  }
  return std::make_tuple(thread_count, batch_thread_count);
}

  void
Inference::reinitialize(const ChatOptions& opt, const struct llama_model* model)
{
  fildesh::ofstream eout("/dev/stderr");

  const auto* sampling = std::get_if<rendezllama::inference::Sampling>(&opt.infer_via);
  assert(sampling);
  auto seed = sampling->seed;
  if (smpl_ || seed < 0) {
    // We're retrying or just don't have a fixed seed, so we should reseed.
    seed = new_sampling_seed();
  }
  std::tie(thread_count_, batch_thread_count_) = infer_thread_counts(opt);
  if (smpl_) {
    llama_sampler_free(smpl_);
    eout.open("/dev/null");
  }
  token_count_ = 0;
  auto smpl_param = llama_sampler_chain_default_params();
  smpl_ = llama_sampler_chain_init(smpl_param);

  for (const auto& adjust_via : sampling->adjust_thru) {
    apply_sampler_chain(smpl_, adjust_via, model, seed, eout);
  }

  if (std::get_if<rendezllama::inference::Probability>(&sampling->pick_via)) {
    llama_sampler_chain_add(smpl_, llama_sampler_init_dist(seed));
  }
  else if (std::get_if<rendezllama::inference::Determinism>(&sampling->pick_via)) {
    llama_sampler_chain_add(smpl_, llama_sampler_init_greedy());
  }
  else if (const auto* adaptive_p = std::get_if<rendezllama::inference::AdaptiveP>(&sampling->pick_via)) {
    adaptive_p_sample(smpl_, *adaptive_p, seed);
  }
  else if (const auto* mirostat = std::get_if<rendezllama::inference::Mirostat>(&sampling->pick_via)) {
    mirostat_sample(smpl_, *mirostat, seed, vocabulary_);
  }
  else {
    fildesh_log_error("Missing pick method? Using greedy.");
    llama_sampler_chain_add(smpl_, llama_sampler_init_greedy());
  }
}

  bool
Inference::commit_to_context(
    struct llama_context* ctx,
    ChatDisplay& chat_disp,
    ChatTrajectory& chat_traj,
    const ChatOptions& opt,
    const llama_model* model)
{
  assert(!chat_traj.erased_since_eval_ ||
         chat_traj.context_token_count_ < chat_traj.token_count());
  if (chat_traj.erased_since_eval_ || !smpl_) {
    this->reinitialize(opt, model);
  }
  if (chat_traj.context_token_count_ == chat_traj.token_count()) {
    return true;
  }

  chat_traj.maybe_rollforget_within_limit(opt.context_token_limit, vocabulary_);

  // Reset thread count just in case the user reconfigured it.
  llama_set_n_threads(ctx, thread_count_, batch_thread_count_);

  // Clear KV cache past current position just in case the user deleted tokens.
  llama_memory_seq_rm(
      llama_get_memory(ctx),
      0, chat_traj.context_token_count_, -1);

  while (chat_traj.context_token_count_ < chat_traj.token_count()) {
    const unsigned n = std::min(
        opt.batch_count,
        chat_traj.token_count() - chat_traj.context_token_count_);

    chat_disp.show_new(chat_traj.context_token_count_ + n, chat_traj, vocabulary_);

    if (!batch_.token || (unsigned)batch_.n_tokens < n) {
      llama_batch_free(batch_);
      unsigned n_alloc = n;
      if (n_alloc < opt.batch_count) {n_alloc = opt.batch_count;}
      batch_ = llama_batch_init(n_alloc, /*embd=*/0, /*n_seq_max=*/1);
    }
    batch_.n_tokens = n;
    for (unsigned i = 0; i < n; ++i) {
      batch_.token[i] = chat_traj.tokens()[chat_traj.context_token_count_ + i];
      batch_.pos[i] = chat_traj.context_token_count_ + i;
      batch_.n_seq_id[i] = 1;
      batch_.seq_id[i][0] = 0;
      batch_.logits[i] = (i == n - 1);
    }

    const int istat = llama_decode(ctx, batch_);

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
  while (token_count_ < chat_traj.token_count()) {
    Vocabulary::Token_id token_id = chat_traj.token_at(token_count_);
    llama_sampler_accept(smpl_, token_id);
    token_count_ += 1;
  }
  return true;
}

  void
Inference::sample_to_trajectory(
    ChatTrajectory& chat_traj,
    struct llama_context* ctx,
    bool preventing_newline)
{
  float* logits = llama_get_logits(ctx);
  if (preventing_newline) {
    // Zero probability for message-ending tokens when requested.
    logits[vocabulary_.eos_token_id()] = 0;
    logits[vocabulary_.newline_token_id()] = 0;
  }

  std::vector<llama_token_data> candidates;
  candidates.resize(vocabulary_.cardinality());
  for (llama_token i = 0; i < (llama_token)candidates.size(); ++i) {
    candidates[i] = llama_token_data{
      i, logits[i], 0.0f,
    };
  }
  logits = NULL;
  llama_token_data_array candidates_data[1] = {{
    candidates.data(),
    candidates.size(),
    /*selected=*/0,
    /*sorted=*/false,
  }};
  llama_sampler_apply(smpl_, candidates_data);
  chat_traj.push_back(candidates[candidates_data->selected].id);
  llama_sampler_accept(smpl_, chat_traj.token());
  token_count_ += 1;
}

  void
Inference::sample_to_trajectory(
    ChatTrajectory& chat_traj,
    struct llama_context* ctx,
    int batch_idx)
{
  float* logits = llama_get_logits_ith(ctx, batch_idx);
  // Note: We don't support preventing_newline here yet, but usually not needed for speculation verification.

  std::vector<llama_token_data> candidates;
  candidates.resize(vocabulary_.cardinality());
  for (llama_token i = 0; i < (llama_token)candidates.size(); ++i) {
    candidates[i] = llama_token_data{
      i, logits[i], 0.0f,
    };
  }
  logits = NULL;
  llama_token_data_array candidates_data[1] = {{
    candidates.data(),
    candidates.size(),
    /*selected=*/0,
    /*sorted=*/false,
  }};
  llama_sampler_apply(smpl_, candidates_data);
  chat_traj.push_back(candidates[candidates_data->selected].id);
  llama_sampler_accept(smpl_, chat_traj.token());
  token_count_ += 1;
}

static
  void
find_ngram_candidates(
    std::vector<llama_token>& candidates,
    const std::vector<llama_token>& tokens,
    unsigned n_gram_len,
    unsigned candidate_limit)
{
  candidates.clear();
  if (tokens.size() < n_gram_len) { return; }

  // Simple backward search
  size_t n = tokens.size();
  for (size_t i = n - 1 - n_gram_len; i > 0; --i) { // i is the index of the last token of the match candidate
      // We want tokens[i - n_gram_len + 1 ... i] == tokens[n - n_gram_len ... n - 1]
      bool match = true;
      for (size_t j = 0; j < n_gram_len; ++j) {
          if (tokens[i - j] != tokens[n - 1 - j]) {
              match = false;
              break;
          }
      }
      if (match) {
          // Found match ending at i.
          // Candidate tokens start at i + 1.
          for (size_t k = 0; k < candidate_limit; ++k) {
              if (i + 1 + k < n) { // Ensure we don't go past current end (though finding self is weird if we search backwards enough)
                 // Actually we can grab from history.
                 if (i + 1 + k < tokens.size()) {
                     candidates.push_back(tokens[i + 1 + k]);
                 }
              }
          }
          if (!candidates.empty()) return;
      }
  }
}

  bool
Inference::generate_next_tokens(
    struct llama_context* ctx,
    ChatDisplay& chat_disp,
    ChatTrajectory& chat_traj,
    const ChatOptions& opt,
    const llama_model* model,
    unsigned n_tokens)
{
  if (!this->commit_to_context(ctx, chat_disp, chat_traj, opt, model)) {
    return false;
  }

  std::vector<llama_token> draft_candidates;

  for (unsigned i = 0; i < n_tokens; ++i) {
    // 1. We start with a token that has been sampled/accepted but not decoded.
    // The previous loop iteration or commit_to_context left us in this state.
    // chat_traj.token() is the last token (T).

    // Draft candidates
    draft_candidates.clear();
    const unsigned kDraftMax = 5;
    // Only draft if we have enough context and batch space
    if (chat_traj.context_token_count_ + kDraftMax + 1 < opt.context_token_limit &&
        opt.batch_count >= kDraftMax + 1)
    {
       find_ngram_candidates(draft_candidates, chat_traj.tokens(), 2, kDraftMax);
    }

    // Ensure batch size
    unsigned required_batch = 1 + draft_candidates.size();
    // We assume batch_ has capacity of at least opt.batch_count (set by commit_to_context or previous init).
    // Reallocate only if we exceed that or if batch_ is null.
    if (!batch_.token || opt.batch_count < required_batch) {
      if (batch_.token) llama_batch_free(batch_);
      unsigned n_alloc = std::max(opt.batch_count, required_batch);
      batch_ = llama_batch_init(n_alloc, /*embd=*/0, /*n_seq_max=*/1);
    }

    // Prepare batch: [T, C1, C2, ...]
    batch_.n_tokens = required_batch;
    batch_.token[0] = chat_traj.token();
    batch_.pos[0] = chat_traj.context_token_count_;
    batch_.n_seq_id[0] = 1;
    batch_.seq_id[0][0] = 0;
    batch_.logits[0] = true;

    for (size_t k = 0; k < draft_candidates.size(); ++k) {
        batch_.token[k+1] = draft_candidates[k];
        batch_.pos[k+1] = chat_traj.context_token_count_ + 1 + k;
        batch_.n_seq_id[k+1] = 1;
        batch_.seq_id[k+1][0] = 0;
        batch_.logits[k+1] = true;
    }

    if (llama_decode(ctx, batch_) != 0) {
      fildesh_log_error("Failed to eval.");
      return false;
    }

    // T is definitely decoded.
    chat_traj.context_token_count_ += 1;

    // Verify
    bool divergence_found = false;
    for (size_t k = 0; k < draft_candidates.size(); ++k) {
        // Sample R from logits of batch[k] (which was input T or C(k-1))
        // Expect R == C(k).
        this->sample_to_trajectory(chat_traj, ctx, (int)k);
        // sample_to_trajectory pushed R to chat_traj.

        if (chat_traj.token() == draft_candidates[k]) {
            // Match!
            chat_traj.context_token_count_ += 1; // C(k) is confirmed decoded
            i += 1; // We advanced one extra step in the main generation request
            if (chat_traj.token() == vocabulary_.eos_token_id()) {
                chat_disp.show_new(chat_traj, vocabulary_);
                return true;
            }
        } else {
            // Divergence!
            // R != C(k). We accepted R.
            // But we need to remove C(k)... from KV cache.
            // C(k) was at batch_.pos[k+1].
            llama_memory_seq_rm(llama_get_memory(ctx), 0, batch_.pos[k+1], -1);
            divergence_found = true;
            break;
        }
    }

    if (!divergence_found) {
        // All candidates matched.
        // We still have the output from the last candidate (batch index `draft_candidates.size()`).
        // Use it to generate the *next* token (start of next iteration).
        this->sample_to_trajectory(chat_traj, ctx, (int)draft_candidates.size());
        // This new token is NOT decoded yet.
    }
    else {
         // Divergence found. We accepted R (the divergence).
         // R is NOT decoded.
         // We removed invalid KV.
         // We are ready for next iter.
    }

    if (chat_traj.token() == vocabulary_.eos_token_id()) {
      chat_disp.show_new(chat_traj, vocabulary_);
      return true;
    }

    chat_disp.show_new(chat_traj, vocabulary_);

    if (chat_traj.context_token_count_ >= opt.context_token_limit) {
       // Sync if full
       if (!this->commit_to_context(ctx, chat_disp, chat_traj, opt, model)) {
         return false;
       }
    }
  }
  return true;
}

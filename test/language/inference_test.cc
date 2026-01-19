#include "src/chat/display.hh"
#include "src/chat/opt.hh"
#include "src/chat/trajectory.hh"
#include "src/language/inference.hh"
#include "src/language/vocabulary.hh"

#include <cassert>
#include <iostream>
#include <tuple>
#include <vector>

#include <fildesh/fildesh.h>

#include "llama.h"

using rendezllama::ChatDisplay;
using rendezllama::ChatOptions;
using rendezllama::ChatTrajectory;
using rendezllama::Inference;
using rendezllama::Vocabulary;

static
  void
noop_log_callback(enum ggml_log_level level, const char* text, void* user_data)
{
  (void) level;
  (void) text;
  (void) user_data;
}

static void test_antiprompt_suffix() {
  std::set<std::string> antiprompts;
  antiprompts.insert("User:");
  antiprompts.insert("\nUser:");

  // Case: No match
  assert(rendezllama::antiprompt_suffix("Hello World", antiprompts).empty());

  // Case: Exact match
  assert(rendezllama::antiprompt_suffix("User:", antiprompts) == "User:");

  // Case: Suffix match
  assert(rendezllama::antiprompt_suffix("Hello User:", antiprompts) == "User:");

  // Case: Longest match check
  assert(rendezllama::antiprompt_suffix("Hello\nUser:", antiprompts) == "\nUser:");

  // Case: Partial match should fail
  assert(rendezllama::antiprompt_suffix("User", antiprompts).empty());
}

static void inference_test(const std::string& model_filename) {
  llama_log_set(noop_log_callback, NULL);

  ChatOptions opt;
  opt.model_filename = model_filename;
  // Initialize infer_via with default Sampling to avoid assertion failure in Inference::reinitialize.
  opt.infer_via.emplace<rendezllama::inference::Sampling>();

  struct llama_model* model = nullptr;
  struct llama_context* ctx = nullptr;
  std::tie(model, ctx) = rendezllama::make_llama_context(opt);
  assert(model);
  assert(ctx);

  Vocabulary vocabulary(model);
  Inference inference(vocabulary);
  ChatTrajectory chat_traj(vocabulary.bos_token_id());
  ChatDisplay chat_disp;
  // Use /dev/null for display to avoid spamming test output, we check tokens programmatically.
  chat_disp.out_ = open_FildeshOF("/dev/null");

  // Add a simple prompt to start generation.
  chat_traj.tokenize_append("Once upon a time", vocabulary);
  chat_disp.show_new(chat_traj, vocabulary);

  using rendezllama::inference::AdjustViaKind;
  using rendezllama::inference::Sampling;

  std::vector<Sampling> samplings;
  // Temperature
  {
    Sampling s;
    s.adjust_thru.emplace_back(std::in_place_index<AdjustViaKind::temperature>, 0.8f);
    samplings.push_back(s);
  }
  // Top K
  {
    Sampling s;
    s.adjust_thru.emplace_back(std::in_place_index<AdjustViaKind::top_k>, 40u);
    samplings.push_back(s);
  }
  // Top P
  {
    Sampling s;
    s.adjust_thru.emplace_back(std::in_place_index<AdjustViaKind::top_p>, 0.9f);
    samplings.push_back(s);
  }
  // Min P
  {
    Sampling s;
    s.adjust_thru.emplace_back(std::in_place_index<AdjustViaKind::min_p>, 0.05f);
    samplings.push_back(s);
  }
   // Typical P
  {
    Sampling s;
    s.adjust_thru.emplace_back(std::in_place_index<AdjustViaKind::typical_p>, 0.9f);
    samplings.push_back(s);
  }
  // Greedy
  {
    Sampling s;
    s.pick_via = rendezllama::inference::Determinism{};
    samplings.push_back(s);
  }
  // Adaptive P
  {
    Sampling s;
    s.pick_via = rendezllama::inference::AdaptiveP{0.55f, 0.9f};
    samplings.push_back(s);
  }
  // Mirostat V2
  {
    Sampling s;
    s.pick_via = rendezllama::inference::Mirostat{2, 5.0f, 0.1f};
    samplings.push_back(s);
  }
  // Penalties
  {
    Sampling s;
    rendezllama::inference::PenalizeWith p;
    p.window_length = 5;
    p.repetition = 1.1f;
    s.adjust_thru.emplace_back(std::in_place_index<AdjustViaKind::penalize_with>, p);
    samplings.push_back(s);
  }
  // Dry
  {
    Sampling s;
    rendezllama::inference::Dry d;
    d.multiplier = 0.8f;
    d.base = 1.75f;
    d.allowed_length = 2;
    d.window_length = 0; // Default.
    s.adjust_thru.emplace_back(std::in_place_index<AdjustViaKind::dry>, d);
    samplings.push_back(s);
  }
  // XTC
  {
    Sampling s;
    rendezllama::inference::Xtc x;
    x.threshold = 0.1f;
    x.probability = 0.5f;
    s.adjust_thru.emplace_back(std::in_place_index<AdjustViaKind::xtc>, x);
    samplings.push_back(s);
  }

  // Iterate through different sampling options.
  bool all_good = true;
  for (const auto& sampling : samplings) {
    opt.infer_via = sampling;
    if (!inference.commit_to_context(ctx, chat_disp, chat_traj, opt, model)) {
      all_good = false;
      break;
    }
    inference.sample_to_trajectory(chat_traj, ctx, false);
    chat_disp.show_new(chat_traj, vocabulary);
  }
  assert(all_good);

  llama_free(ctx);
  llama_model_free(model);
}

int main(int argc, char** argv)
{
  rendezllama::GlobalScope rendezllama_global_scope;
  assert(argc == 2);
  test_antiprompt_suffix();
  inference_test(argv[1]);
  return 0;
}

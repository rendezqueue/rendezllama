#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include <fildesh/ostream.hh>
#include <fildesh/string.hh>

#include "src/chat/display.hh"
#include "src/chat/opt.hh"
#include "src/chat/trajectory.hh"
#include "src/language/inference.hh"
#include "src/language/vocabulary.hh"

using rendezllama::ChatDisplay;
using rendezllama::ChatOptions;
using rendezllama::ChatTrajectory;
using rendezllama::GlobalScope;
using rendezllama::Inference;
using rendezllama::Vocabulary;

static void noop_log_callback(enum ggml_log_level level, const char* text, void* user_data) {
  (void) level;
  (void) text;
  (void) user_data;
}

static void print_usage(const char* argv0) {
  fprintf(stderr, "usage: %s --model <model_path> [--context_length <n>]\n", argv0);
}

int main(int argc, char** argv) {
  ChatOptions opt;
  opt.model_filename = "";
  opt.context_token_limit = 2048;

  // Use deterministic (greedy) sampling for reproducibility
  auto& sampling = std::get<rendezllama::inference::Sampling>(opt.infer_via);
  sampling.pick_via = rendezllama::inference::Determinism();

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--model") {
      if (++i < argc) {
        opt.model_filename = argv[i];
      } else {
        print_usage(argv[0]);
        return 1;
      }
    } else if (arg == "--context_length") {
      if (++i < argc) {
        opt.context_token_limit = std::stoi(argv[i]);
      } else {
        print_usage(argv[0]);
        return 1;
      }
    }
  }

  if (opt.model_filename.empty()) {
    print_usage(argv[0]);
    return 1;
  }

  // Suppress llama.cpp logs
  llama_log_set(noop_log_callback, NULL);

  GlobalScope global_scope;

  auto [model, ctx] = rendezllama::make_llama_context(opt);
  if (!model || !ctx) {
    return 1;
  }

  Vocabulary vocabulary(model);
  ChatTrajectory chat_traj(vocabulary.bos_token_id());
  ChatDisplay chat_disp;
  chat_disp.out_ = open_FildeshOF("/dev/stdout");

  Inference inference(vocabulary);

  std::vector<Vocabulary::ChatMessage> messages;
  std::vector<char> formatted;
  size_t old_len = 0;

  while (true) {
    putstr_FildeshO(chat_disp.out_, "> ");
    flush_FildeshO(chat_disp.out_);

    std::string user_input;
    if (!std::getline(std::cin, user_input) || user_input.empty()) {
      break;
    }

    messages.push_back({"user", user_input});

    // Apply template to get user message + assistant prefix
    int new_len = vocabulary.chat_apply_template(messages, formatted, true);
    if (new_len < 0) {
      fprintf(stderr, "Failed to apply chat template\n");
      break;
    }

    // Append new parts to trajectory
    if ((size_t)new_len > old_len) {
      std::string delta(formatted.begin() + old_len, formatted.begin() + new_len);

      // Tokenize with special=true because chat template output contains special tokens (e.g. <s>, [INST])
      // Vocabulary::tokenize_to disables special tokens, so we use llama_tokenize directly.
      std::vector<llama_token> tokens(delta.size() + 1);
      int n_tokens = llama_tokenize(llama_model_get_vocab(model), delta.c_str(), delta.size(), tokens.data(), tokens.size(), false, true);
      if (n_tokens < 0) {
        tokens.resize(-n_tokens);
        n_tokens = llama_tokenize(llama_model_get_vocab(model), delta.c_str(), delta.size(), tokens.data(), tokens.size(), false, true);
      }
      tokens.resize(n_tokens);

      std::vector<Vocabulary::Token_id> trajectory_tokens;
      trajectory_tokens.reserve(tokens.size());
      for (auto t : tokens) {
        trajectory_tokens.push_back(t);
      }

      chat_traj.insert_all_at(chat_traj.token_count(), trajectory_tokens);
      // Advance display counter so we don't echo user input/template parts
      chat_traj.display_token_count_ = chat_traj.token_count();
      old_len = new_len;
    }

    if (!inference.commit_to_context(ctx, chat_disp, chat_traj, opt, model)) {
      break;
    }

    // Generate response
    std::string response_text;
    while (true) {
      inference.sample_to_trajectory(chat_traj, ctx, false);
      Vocabulary::Token_id token = chat_traj.token();

      if (token == vocabulary.eos_token_id()) {
        break;
      }

      chat_disp.show_new(chat_traj, vocabulary);

      fildesh::ostringstream oss;
      vocabulary.detokenize_to(oss, token);
      response_text += oss.view();

      if (!inference.commit_to_context(ctx, chat_disp, chat_traj, opt, model)) {
        break;
      }
    }
    putc_FildeshO(chat_disp.out_, '\n');
    flush_FildeshO(chat_disp.out_);

    messages.push_back({"assistant", response_text});

    // Update old_len to include the assistant message we just generated
    new_len = vocabulary.chat_apply_template(messages, formatted, false);
    if (new_len >= 0) {
      old_len = new_len;
    }
  }

  llama_free(ctx);
  llama_model_free(model);

  return 0;
}

#ifndef RENDEZLLAMA_LANGUAGE_INFERENCE_HH_
#define RENDEZLLAMA_LANGUAGE_INFERENCE_HH_
#include <ostream>
#include <string>
#include <tuple>
#include <set>
#include <vector>

#include "llama.h"

namespace rendezllama {

struct ChatOptions;
class ChatDisplay;
class ChatGuide;
class ChatTrajectory;
class Vocabulary;

class Inference {
 public:
  explicit Inference(const Vocabulary& vocabulary);
  Inference(const Inference&) = delete;
  Inference(Inference&&) = delete;
  ~Inference();
  Inference& operator=(const Inference&) = delete;
  Inference& operator=(Inference&&) = delete;

 private:
  void reinitialize(
      const ChatOptions& opt,
      const struct llama_model* model);

 public:
  bool commit_to_context(
      struct llama_context* ctx,
      ChatDisplay& chat_disp,
      ChatTrajectory& chat_traj,
      const ChatOptions& opt,
      const llama_model* model);
  void sample_to_trajectory(
      ChatTrajectory& chat_traj,
      struct llama_context* ctx,
      bool preventing_newline);
  void sample_to_trajectory(
      ChatTrajectory& chat_traj,
      struct llama_context* ctx,
      int batch_idx);
  bool generate_next_tokens(
      struct llama_context* ctx,
      ChatDisplay& chat_disp,
      ChatTrajectory& chat_traj,
      const ChatOptions& opt,
      const llama_model* model,
      unsigned n_tokens);

 private:
  unsigned thread_count_ = 0;
  unsigned batch_thread_count_ = 0;
  llama_sampler* smpl_ = nullptr;
  llama_batch batch_ = {0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
  size_t token_count_ = 0;
  const Vocabulary& vocabulary_;
};

const std::string&
antiprompt_suffix(
    std::string_view text,
    const std::set<std::string>& antiprompts);
void
augment_tokenize_chat_input(
    ChatGuide& chat_guide,
    ChatTrajectory& chat_traj,
    bool& prevent_subsequent_newline,
    std::string s,
    const Vocabulary& vocabulary,
    const ChatOptions& opt);


std::tuple<struct llama_model*, struct llama_context*>
make_llama_context(ChatOptions& opt);

}  // namespace rendezllama
#endif

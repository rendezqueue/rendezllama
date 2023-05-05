#ifndef RENDEZLLAMA_CHAT_TRAJECTORY_HH_
#define RENDEZLLAMA_CHAT_TRAJECTORY_HH_
#include <vector>

struct FildeshO;
struct llama_context;

namespace rendezllama {

class ChatTrajectory {
 public:
  typedef int Token_id;
  typedef unsigned size_type;

 public:
  ChatTrajectory();
  ~ChatTrajectory();

  size_type token_count() const {return token_values_.size();}
  void push_back(Token_id token_id);
  void insert_all_at(size_type i, const std::vector<Token_id>& a);
  void erase_range(size_type beg, size_type end);
  void erase_all_at(size_type beg) {this->erase_range(beg, this->token_count());}
  void rollforget(size_type end, const struct llama_context* ctx);

  Token_id token() const {return token_values_.back();}
  const Token_id& token_at(size_type i) const {return token_values_[i];}

  float mirostat_mu() const {return mirostat_mu_values_.back();}
  float& mirostat_mu() {return mirostat_mu_values_.back();}
  const float& mirostat_mu_at(unsigned i) const {return mirostat_mu_values_[i];}
  float& mirostat_mu_at(unsigned i) {return mirostat_mu_values_[i];}

  const std::vector<Token_id>& tokens() const {return token_values_;}

 private:
  std::vector<Token_id> token_values_;
  std::vector<float> mirostat_mu_values_;
 public:
  FildeshO* transcript_out_ = nullptr;
  size_type context_token_count_ = 0;
  size_type priming_token_count_ = 0;
};

}  // namespace rendezllama
#endif


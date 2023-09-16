#ifndef RENDEZLLAMA_CHAT_TRAJECTORY_HH_
#define RENDEZLLAMA_CHAT_TRAJECTORY_HH_
#include <vector>

#include "src/language/vocabulary.hh"

namespace rendezllama {

class ChatTrajectory {
 public:
  typedef Vocabulary::Token_id Token_id;
  typedef unsigned size_type;

 public:
  explicit ChatTrajectory(Token_id);
  ~ChatTrajectory();

  size_type token_count() const {return token_values_.size();}
  void push_back(Token_id token_id);
  void insert_all_at(size_type i, const std::vector<Token_id>& a);
  void erase_range(size_type beg, size_type end);
  void erase_all_at(size_type beg) {this->erase_range(beg, this->token_count());}
  void rollforget(size_type end, const Vocabulary& vocabulary);

  Token_id token() const {return token_values_.back();}
  const Token_id& token_at(size_type i) const {return token_values_[i];}

  float mirostat_mu() const {return mirostat_mu_values_.back();}
  float& mirostat_mu() {return mirostat_mu_values_.back();}
  const float& mirostat_mu_at(unsigned i) const {return mirostat_mu_values_[i];}
  float& mirostat_mu_at(unsigned i) {return mirostat_mu_values_[i];}

  unsigned line_prefix_index() const {return line_prefix_indices_.back();}
  unsigned& line_prefix_index() {return line_prefix_indices_.back();}
  unsigned line_prefix_index_at(unsigned i) const {return line_prefix_indices_[i];}
  unsigned& line_prefix_index_at(unsigned i) {return line_prefix_indices_[i];}

  const std::vector<Token_id>& tokens() const {return token_values_;}

 private:
  std::vector<Token_id> token_values_;
  std::vector<float> mirostat_mu_values_;
  std::vector<unsigned> line_prefix_indices_;
 public:
  FildeshO* transcript_out_ = nullptr;
  size_type display_token_count_ = 0;
  size_type context_token_count_ = 0;
  size_type priming_token_count_ = 0;
  bool erased_since_eval_ = false;
};

}  // namespace rendezllama
#endif


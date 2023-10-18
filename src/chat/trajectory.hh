#ifndef RENDEZLLAMA_CHAT_TRAJECTORY_HH_
#define RENDEZLLAMA_CHAT_TRAJECTORY_HH_

#include <limits>

#include "src/language/vocabulary.hh"

namespace rendezllama {

class ChatTrajectory {
 public:
  typedef Vocabulary::Token_id Token_id;
  typedef unsigned message_prefix_id;
  typedef unsigned size_type;

 public:
  explicit ChatTrajectory(Token_id);
  ~ChatTrajectory();

  size_type token_count() const {return token_ids_.size();}
  void push_back(Token_id token_id);
  void insert_all_at(size_type i, const std::vector<Token_id>& a);
  void tokenize_append(std::string_view s, const Vocabulary& vocabulary);

  void erase_range(size_type beg, size_type end);
  void erase_all_at(size_type beg) {this->erase_range(beg, this->token_count());}
  void rollforget(size_type end, const Vocabulary& vocabulary);

  Token_id token() const {return token_ids_.back();}
  Token_id token_at(size_type i) const {return token_ids_[i];}
  size_type find_token_at(size_type i, Token_id id) const;
  size_type rfind_token_at(size_type i, Token_id id) const;

  float mirostat_mu() const {return mirostat_mu_values_.back();}
  float& mirostat_mu() {return mirostat_mu_values_.back();}
  const float& mirostat_mu_at(unsigned i) const {return mirostat_mu_values_[i];}
  float& mirostat_mu_at(unsigned i) {return mirostat_mu_values_[i];}

  void tokenize_append_message_prefix(
      message_prefix_id id,
      std::string_view s,
      const Vocabulary& vocabulary);
  static message_prefix_id unknown_message_prefix_id() {
    return std::numeric_limits<message_prefix_id>::max()-1;
  }
  static message_prefix_id not_a_message_prefix_id() {
    return std::numeric_limits<message_prefix_id>::max();
  }
  size_type rfind_message_prefix_at(size_type i) const;
  size_type rfind_message_prefix_begin_at(size_type i) const;
  size_type rfind_last_message_prefix_end_at(size_type i) const;
  message_prefix_id last_message_prefix_id_at(size_type i) const;
  void assign_range_message_prefix_id(
      message_prefix_id id,
      size_type beg, size_type end);

  const std::vector<Token_id>& tokens() const {return token_ids_;}

 private:
  std::vector<Token_id> token_ids_;
  std::vector<float> mirostat_mu_values_;
  std::vector<unsigned> message_prefix_ids_;
 public:
  FildeshO* transcript_out_ = nullptr;
  size_type display_token_count_ = 0;
  size_type context_token_count_ = 0;
  size_type priming_token_count_ = 0;
  message_prefix_id message_prefix_id_ = ChatTrajectory::unknown_message_prefix_id();
  bool erased_since_eval_ = false;
};

}  // namespace rendezllama
#endif


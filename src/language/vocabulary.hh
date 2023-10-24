#ifndef RENDEZLLAMA_LANGUAGE_VOCABULARY_HH_
#define RENDEZLLAMA_LANGUAGE_VOCABULARY_HH_
#include <ostream>
#include <string>
#include <vector>

struct FildeshO;
struct llama_model;

namespace rendezllama {

class Vocabulary {
 public:
  typedef int Token_id;

 public:
  explicit Vocabulary(const llama_model* model);

  Token_id bos_token_id() const;
  Token_id eos_token_id() const;
  Token_id newline_token_id() const;
  unsigned cardinality() const;

  char last_char_of(Token_id token_id) const;

  void detokenize_to(FildeshO* out, Token_id token_id) const;
  void detokenize_to(FildeshO* out, const Token_id* ids, size_t n) const {
    for (size_t i = 0; i < n; ++i) {
      this->detokenize_to(out, ids[i]);
    }
  }
  void detokenize_to(std::ostream& out, const Token_id* ids, size_t n) const;
  void detokenize_to(std::ostream& out, Token_id token_id) const {
    this->detokenize_to(out, &token_id, 1);
  }

  void tokenize_to(std::vector<Token_id>& tokens, std::string_view text) const;

 private:
  const llama_model* model_;
};

class GlobalScope {
 public:
  GlobalScope();
  ~GlobalScope();
};

}  // namespace rendezllama
#endif


#ifndef RENDEZLLAMA_LLM_VOCABULARY_HH_
#define RENDEZLLAMA_LLM_VOCABULARY_HH_
#include <string>
#include <ostream>

struct FildeshO;
struct llama_context;

namespace rendezllama {

class Vocabulary {
 public:
  typedef int Token_id;

 public:
  explicit Vocabulary(const llama_context* ctx);

  Token_id bos_token_id() const;
  Token_id eos_token_id() const;
  Token_id newline_token_id() const;

  char last_char_of(Token_id token_id) const;

  void detokenize_to(FildeshO* out, Token_id token_id) const;
  void detokenize_to(std::ostream& out, Token_id token_id) const;
  void detokenize_to(std::string& out, Token_id token_id) const;

 private:
  const llama_context* ctx_;
};

}  // namespace rendezllama
#endif


#include "src/language/vocabulary.hh"

#include <algorithm>
#include <cassert>
#include <cstring>

#include <fildesh/ostream.hh>
#include <fildesh/string.hh>

#include "llama.h"

using rendezllama::Vocabulary;
typedef Vocabulary::Token_id Token_id;

Vocabulary::Vocabulary(const llama_model* model)
  : model_(model)
{
  if (!model_) {return;}

  int n = llama_tokenize(
      model_, "\n", 1, &newline_token_id_, 1,
      /*add_bos=*/false,
      /*special=*/true);
  assert(n == 1 && "need a unique newline token");
}

Token_id Vocabulary::bos_token_id() const {
  if (!model_) {return 0;}
  return llama_token_bos(model_);
}
Token_id Vocabulary::eos_token_id() const {
  if (!model_) {return 0;}
  return llama_token_eos(model_);
}
Token_id Vocabulary::newline_token_id() const {
  if (!model_) {return 0;}
  return newline_token_id_;
}

unsigned Vocabulary::cardinality() const {
  if (!model_) {return 1;}
  return llama_n_vocab(model_);
}

char Vocabulary::last_char_of(Token_id token_id) const {
  fildesh::ostringstream oss;
  this->detokenize_to(oss.c_struct(), token_id);
  const std::string_view s = oss.view();
  if (!s.empty()) {
    return s[s.size()-1];
  }
  return '\0';
}

  void
Vocabulary::detokenize_to(FildeshO* out, Token_id token_id) const
{
  const size_t attempt_size = allocated_size_of_FildeshO(out) - out->size;
  char* s = grow_FildeshO(out, attempt_size);

  int n = llama_token_to_piece(model_, token_id, s, attempt_size);
  if (n >= 0) {
    out->size -= (attempt_size - n);
  } else {
    n = -n;
    out->size -= attempt_size;
    s = grow_FildeshO(out, n);
    n = llama_token_to_piece(model_, token_id, s, n);
  }
}

  void
Vocabulary::detokenize_to(std::ostream& out, const Token_id* ids, size_t n) const
{
  fildesh::ostreambuf* outbuf = dynamic_cast<fildesh::ostreambuf*>(out.rdbuf());
  if (outbuf) {
    this->detokenize_to(outbuf->c_struct(), ids, n);
  }
  else {
    fildesh::ostringstream oss;
    this->detokenize_to(oss.c_struct(), ids, n);
    out << oss.view();
  }
}

  void
Vocabulary::tokenize_to(
    std::vector<Token_id>& tokens,
    std::string_view text) const
{
  std::string s = "\n";
  s += text;
  tokens.resize(1 + s.size());
  int n = llama_tokenize(
      model_,
      s.data(), s.size(),
      tokens.data(), tokens.size(),
      /*add_bos=*/false,
      /*special=*/false);
  assert(n >= 1);
  tokens.resize((size_t)n);
  std::vector<Token_id>::iterator it = std::find(
      tokens.begin(), tokens.end(),
      this->newline_token_id());
  assert(it != tokens.end());
  tokens.erase(tokens.begin(), it+1);
}

rendezllama::GlobalScope::GlobalScope() {
  llama_backend_init(/*numa=*/false);
}

rendezllama::GlobalScope::~GlobalScope() {
  llama_backend_free();
}

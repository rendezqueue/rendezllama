#include "src/language/vocabulary.hh"

#include <algorithm>
#include <cassert>
#include <cstring>

#include <fildesh/fildesh.h>

#include "llama.h"

using rendezllama::Vocabulary;
typedef Vocabulary::Token_id Token_id;

Vocabulary::Vocabulary(const llama_context* ctx)
  : ctx_(ctx)
{}

Token_id Vocabulary::bos_token_id() const {
  if (!ctx_) {return 0;}
  return llama_token_bos(ctx_);
}
Token_id Vocabulary::eos_token_id() const {
  if (!ctx_) {return 0;}
  return llama_token_eos(ctx_);
}
Token_id Vocabulary::newline_token_id() const {
  if (!ctx_) {return 0;}
  return llama_token_nl(ctx_);
}

unsigned Vocabulary::cardinality() const {
  if (!ctx_) {return 1;}
  return llama_n_vocab(llama_get_model(ctx_));
}

char Vocabulary::last_char_of(Token_id token_id) const {
  std::string s;
  this->detokenize_to(s, token_id);
  if (!s.empty()) {
    return s[s.size()-1];
  }
  return '\0';
}

void Vocabulary::detokenize_to(FildeshO* out, Token_id token_id) const {
  const size_t attempt_size = 8;
  char* s = grow_FildeshO(out, attempt_size);

  const llama_model* model = llama_get_model(ctx_);
  int n = llama_token_to_piece(model, token_id, s, attempt_size);
  if (n >= 0) {
    out->size -= (attempt_size - n);
  } else {
    n = -n;
    out->size -= attempt_size;
    s = grow_FildeshO(out, n);
    n = llama_token_to_piece(model, token_id, s, n);
  }
}

void Vocabulary::detokenize_to(std::ostream& out, Token_id token_id) const {
  std::string s;
  this->detokenize_to(s, token_id);
  out << s;
}

void Vocabulary::detokenize_to(std::string& out, Token_id token_id) const {
  const size_t attempt_size = 8;
  out.resize(attempt_size);

  const llama_model* model = llama_get_model(ctx_);
  int n = llama_token_to_piece(model, token_id, &out[0], attempt_size);
  if (n >= 0) {
    out.resize(n);
  } else {
    n = -n;
    out.resize(n);
    n = llama_token_to_piece(model, token_id, &out[0], n);
  }
}

  void
Vocabulary::tokenize_to(
    std::vector<Token_id>& tokens,
    const std::string& text) const
{
  std::string s = "\n";
  s.append(text);
  tokens.resize(1 + s.size());
  const llama_model* model = llama_get_model(ctx_);
  int n = llama_tokenize(
      model,
      &s[0], s.size(),
      &tokens[0], tokens.size(),
      false);
  assert(n >= 1);
  tokens.resize((size_t)n);
  std::vector<Token_id>::iterator it = std::find(
      tokens.begin(), tokens.end(),
      this->newline_token_id());
  assert(it != tokens.end());
  tokens.erase(tokens.begin(), it+1);
}

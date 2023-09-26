#include "src/language/vocabulary.hh"

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
  return llama_token_bos();
}
Token_id Vocabulary::eos_token_id() const {
  if (!ctx_) {return 0;}
  return llama_token_eos();
}
Token_id Vocabulary::newline_token_id() const {
  if (!ctx_) {return 0;}
  return llama_token_nl();
}

char Vocabulary::last_char_of(Token_id token_id) const {
  const char* s = llama_token_to_str(ctx_, token_id);
  if (s) {
    return s[strlen(s)-1];
  }
  return '\0';
}

void Vocabulary::detokenize_to(FildeshO* out, Token_id token_id) const {
  const char* s = llama_token_to_str(ctx_, token_id);
  if (s) {
    puts_FildeshO(out, s);
  }
}

void Vocabulary::detokenize_to(std::ostream& out, Token_id token_id) const {
  const char* s = llama_token_to_str(ctx_, token_id);
  if (s) {
    out << s;
  }
}

void Vocabulary::detokenize_to(std::string& out, Token_id token_id) const {
  const char* s = llama_token_to_str(ctx_, token_id);
  if (s) {
    out = s;
  }
  else {
    out.clear();
  }
}

  void
Vocabulary::tokenize_to(
    std::vector<Token_id>& tokens,
    const std::string& text) const
{
  tokens.resize(text.size());
  int n = llama_tokenize(
      (llama_context*)ctx_,
      text.c_str(),
      &tokens[0], tokens.size(), false);
  assert(n >= 0);
  tokens.resize((size_t)n);
}

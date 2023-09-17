#include "src/language/vocabulary.hh"

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

void Vocabulary::detokenize_to(FildeshO* out, Token_id token_id) const {
  const char* s = llama_token_to_str(ctx_, token_id);
  if (s) {
    puts_FildeshO(out, s);
  }
}

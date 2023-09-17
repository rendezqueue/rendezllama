#include "tokenize.hh"

#include <cassert>
#include <cstring>

#include "llama.h"

using rendezllama::Vocabulary;

  void
rendezllama::tokenize_extend(
    std::vector<Vocabulary::Token_id>& tokens,
    struct llama_context* ctx,
    const std::string& text)
{
  const size_t offset = tokens.size();
  tokens.resize(offset + text.size());
  int n = llama_tokenize(
      ctx, text.c_str(), &tokens[offset], text.size(), false);
  assert(n >= 0);
  tokens.resize(offset + (size_t)n);
}

  size_t
rendezllama::prev_newline_start_index(
    const Vocabulary& vocabulary,
    const std::vector<Vocabulary::Token_id>& tokens,
    size_t offset)
{
  const llama_token newline_token = vocabulary.newline_token_id();
  size_t i = offset;
  if (i > 0) {
    i -= 1;
  }
  while (i > 0) {
    i -= 1;
    if (newline_token == tokens[i]) {
      return i+1;
    }
  }
  return 0;
}

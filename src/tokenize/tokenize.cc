#include "tokenize.hh"

#include <cassert>
#include <cstring>

#include "src/language/vocabulary.hh"

  void
rendezllama::tokenize_extend(
    std::vector<llama_token>& tokens,
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

  bool
rendezllama::token_endswith(const struct llama_context* ctx, llama_token token_id, char c)
{
  const char* s = llama_token_to_str(ctx, token_id);
  if (!s) {return false;}
  s = strrchr(s, c);
  return (s && s[1] == '\0');
}

  size_t
rendezllama::prev_newline_start_index(
    const struct llama_context* ctx,
    const std::vector<llama_token>& tokens,
    size_t offset)
{
  const Vocabulary vocabulary(ctx);
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

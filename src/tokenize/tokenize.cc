#include "tokenize.hh"

#include <cassert>
#include <cstring>

  llama_token
rendezllama::newline_token(const struct llama_context* ctx)
{
  llama_token token_id = 0;
  int n = llama_tokenize(
      const_cast<struct llama_context*>(ctx),
      "\n", &token_id, 1, false);
  assert(n >= 0);
  return token_id;
}

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
  const llama_token newline_token = rendezllama::newline_token(ctx);
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

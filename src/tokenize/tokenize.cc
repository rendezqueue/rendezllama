#include "tokenize.hh"

#include <cassert>
#include <cstring>

  llama_token
rendezllama::newline_token(struct llama_context* ctx)
{
  llama_token token_id = 0;
  int n = llama_tokenize(ctx, "\n", &token_id, 1, false);
  assert(n >= 0);
  return token_id;
}

  void
rendezllama::print_tokens(
    std::ostream& out,
    std::vector<llama_token>::iterator first,
    std::vector<llama_token>::iterator last,
    const struct llama_context* ctx)
{
  if (!out.good()) {return;}
  while (first != last) {
    const char* s = llama_token_to_str(const_cast<struct llama_context*>(ctx), *first);
    out << s;
    ++ first;
  }
  out.flush();
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
  const char* s = llama_token_to_str(
      const_cast<struct llama_context*>(ctx), token_id);
  if (!s) {return false;}
  s = strrchr(s, c);
  return (s && s[1] == '\0');
}

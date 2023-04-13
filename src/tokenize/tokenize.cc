#include "tokenize.hh"

#include <cassert>

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

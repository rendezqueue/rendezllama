#ifndef RENDEZLLAMA_TOKENIZE_HH_
#define RENDEZLLAMA_TOKENIZE_HH_
#include <ostream>
#include <string>
#include <vector>

#include "llama.h"

namespace rendezllama {

llama_token
newline_token(struct llama_context* ctx);
void
print_tokens(
    std::ostream& out,
    std::vector<llama_token>::iterator first,
    std::vector<llama_token>::iterator last,
    const struct llama_context* ctx);
void
tokenize_extend(
    std::vector<llama_token>& tokens,
    struct llama_context* ctx,
    const std::string& text);
bool
token_endswith(const struct llama_context* ctx, llama_token token_id, char c);

}  // namespace rendezllama
#endif

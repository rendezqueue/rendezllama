#ifndef RENDEZLLAMA_TOKENIZE_HH_
#define RENDEZLLAMA_TOKENIZE_HH_
#include <ostream>
#include <string>
#include <vector>

#include "llama.h"

namespace rendezllama {

llama_token
newline_token(const struct llama_context* ctx);
void
tokenize_extend(
    std::vector<llama_token>& tokens,
    struct llama_context* ctx,
    const std::string& text);
bool
token_endswith(const struct llama_context* ctx, llama_token token_id, char c);
size_t
prev_newline_start_index(const struct llama_context* ctx,
                         const std::vector<llama_token>& tokens,
                         size_t offset);

}  // namespace rendezllama
#endif

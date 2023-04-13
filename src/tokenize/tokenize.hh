#ifndef RENDEZLLAMA_TOKENIZE_HH_
#define RENDEZLLAMA_TOKENIZE_HH_
#include <string>
#include <vector>

#include "llama.h"

namespace rendezllama {

void
tokenize_extend(
    std::vector<llama_token>& tokens,
    struct llama_context* ctx,
    const std::string& text);

}  // namespace rendezllama
#endif

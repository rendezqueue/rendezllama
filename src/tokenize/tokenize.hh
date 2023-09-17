#ifndef RENDEZLLAMA_TOKENIZE_HH_
#define RENDEZLLAMA_TOKENIZE_HH_
#include <vector>

#include "src/language/vocabulary.hh"

namespace rendezllama {

void
tokenize_extend(
    std::vector<Vocabulary::Token_id>& tokens,
    struct llama_context* ctx,
    const std::string& text);
size_t
prev_newline_start_index(const Vocabulary& vocabulary,
                         const std::vector<Vocabulary::Token_id>& tokens,
                         size_t offset);

}  // namespace rendezllama
#endif

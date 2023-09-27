#ifndef RENDEZLLAMA_TOKENIZE_HH_
#define RENDEZLLAMA_TOKENIZE_HH_
#include <vector>

#include "src/language/vocabulary.hh"

namespace rendezllama {

size_t
prev_newline_start_index(const Vocabulary& vocabulary,
                         const std::vector<Vocabulary::Token_id>& tokens,
                         size_t offset);

}  // namespace rendezllama
#endif

#include "src/language/vocabulary.hh"

#include <algorithm>
#include <cassert>
#include <cstring>

#include <fildesh/ostream.hh>
#include <fildesh/string.hh>

#include "llama.h"

using rendezllama::Vocabulary;
typedef Vocabulary::Token_id Token_id;

Vocabulary::Vocabulary(const llama_model* model)
  : model_(model)
{
  if (!model_) {return;}

  boundary_prefix_ = "☺";
  std::string text = boundary_prefix_ + '\n';
  std::vector<Token_id> tokens(text.size()+1);
  int n = llama_tokenize(
      model_,
      text.data(), text.size(),
      tokens.data(), tokens.size(),
      /*add_bos=*/false,
      /*special=*/false);
  assert(n >= 2 && "need to tokenize boundary prefix");
  newline_token_id_ = tokens[n-1];
  boundary_prefix_tokens_.assign(tokens.begin(), tokens.begin()+(n-1));
}

Token_id Vocabulary::bos_token_id() const {
  if (!model_) {return 0;}
  return llama_token_bos(model_);
}
Token_id Vocabulary::eos_token_id() const {
  if (!model_) {return 0;}
  return llama_token_eos(model_);
}
Token_id Vocabulary::newline_token_id() const {
  if (!model_) {return 0;}
  return newline_token_id_;
}

unsigned Vocabulary::cardinality() const {
  if (!model_) {return 1;}
  return llama_n_vocab(model_);
}

char Vocabulary::last_char_of(Token_id token_id) const {
  fildesh::ostringstream oss;
  this->detokenize_to(oss.c_struct(), token_id);
  const std::string_view s = oss.view();
  if (!s.empty()) {
    return s[s.size()-1];
  }
  return '\0';
}

  void
Vocabulary::detokenize_to(FildeshO* out, Token_id token_id) const
{
  for (const auto& sr : special_tokens_) {
    if (sr.token_id == token_id) {
      *out << sr.alias;
      return;
    }
  }
  const size_t attempt_size = allocated_size_of_FildeshO(out) - out->size;
  char* s = grow_FildeshO(out, attempt_size);

  int n = llama_token_to_piece(model_, token_id, s, attempt_size);
  if (n >= 0) {
    out->size -= (attempt_size - n);
  } else {
    n = -n;
    out->size -= attempt_size;
    s = grow_FildeshO(out, n);
    n = llama_token_to_piece(model_, token_id, s, n);
  }
}

  void
Vocabulary::detokenize_to(std::ostream& out, const Token_id* ids, size_t n) const
{
  fildesh::ostreambuf* outbuf = dynamic_cast<fildesh::ostreambuf*>(out.rdbuf());
  if (outbuf) {
    this->detokenize_to(outbuf->c_struct(), ids, n);
  }
  else {
    fildesh::ostringstream oss;
    this->detokenize_to(oss.c_struct(), ids, n);
    out << oss.view();
  }
}

  Token_id
Vocabulary::tokenize_special(std::string_view s) const
{
  for (auto& sr : special_tokens_) {
    if (sr.alias == s) {
      return sr.token_id;
    }
  }
  Token_id token_id = 0;
  int n = llama_tokenize(
      model_,
      s.data(), s.size(),
      &token_id, 1,
      /*add_bos=*/false,
      /*special=*/true);
  if (n != 1) {
    token_id = this->cardinality();
  }
  return token_id;
}

static
  void
tokenize_append(
    std::vector<Token_id>& tokens,
    std::string_view text,
    const llama_model* model,
    const std::string_view boundary_prefix,
    const std::vector<Token_id>& boundary_prefix_tokens,
    std::string& tmp_s)
{
  if (text.empty()) {return;}
  tmp_s = boundary_prefix;
  tmp_s += text;
  size_t offset = tokens.size();
  tokens.resize(offset + tmp_s.size() + 1);
  int n = llama_tokenize(
      model,
      tmp_s.data(), tmp_s.size(),
      tokens.data()+offset, tokens.size()-offset,
      /*add_bos=*/false,
      /*special=*/false);
  assert(n > 0);
  assert((size_t)n > boundary_prefix_tokens.size());
  tokens.resize(offset + (size_t)n);
  tokens.erase(
      tokens.begin()+offset,
      tokens.begin()+offset+boundary_prefix_tokens.size());
}

  void
Vocabulary::tokenize_to(
    std::vector<Token_id>& tokens,
    std::string_view text) const
{
  tokens.clear();
  std::string_view::size_type end = std::string_view::npos;
  std::vector<size_t> next_indices(special_tokens_.size(), end);
  for (size_t i = 0; i < next_indices.size(); ++i) {
    next_indices[i] = text.find(special_tokens_[i].alias);
    if (next_indices[i] < end) {
      end = next_indices[i];
    }
  }
  std::string tmp_s;
  std::string_view::size_type beg = 0;
  while (end != std::string_view::npos) {
    tokenize_append(tokens, text.substr(beg, end-beg), model_,
                    boundary_prefix_, boundary_prefix_tokens_, tmp_s);
    beg = end;
    end = std::string_view::npos;
    for (size_t i = 0; i < next_indices.size(); ++i) {
      if (beg == next_indices[i]) {
        tokens.push_back(special_tokens_[i].token_id);
        beg += special_tokens_[i].alias.size();
        break;
      }
    }
    for (size_t i = 0; i < next_indices.size(); ++i) {
      if (next_indices[i] < beg) {
        next_indices[i] = text.find(special_tokens_[i].alias, beg);
      }
      if (next_indices[i] < end) {
        end = next_indices[i];
      }
    }
  }
  tokenize_append(tokens, text.substr(beg), model_,
                  boundary_prefix_, boundary_prefix_tokens_, tmp_s);
}

  void
Vocabulary::assign_substitution(std::string_view alias, Token_id token_id)
{
  assert(!alias.empty());
  if (token_id == this->bos_token_id()) {
    bos_token_alias_ = alias;
  }
  if (token_id == this->eos_token_id()) {
    eos_token_alias_ = alias;
  }
  for (auto& sr : special_tokens_) {
    if (sr.alias == alias) {
      sr.token_id = token_id;
      return;
    }
  }
  SubstitutionRule sr;
  sr.alias = alias;
  sr.token_id = token_id;
  special_tokens_.push_back(sr);
}

rendezllama::GlobalScope::GlobalScope() {
  llama_backend_init();
}

rendezllama::GlobalScope::~GlobalScope() {
  llama_backend_free();
}

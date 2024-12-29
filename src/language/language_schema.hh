#ifndef RENDEZLLAMA_LANGUAGE_LANGUAGE_SCHEMA_HH_
#define RENDEZLLAMA_LANGUAGE_LANGUAGE_SCHEMA_HH_

#include <optional>
#include <string>
#include <vector>

#include <fildesh/sxproto.h>

#include "src/language/inference_schema.hh"

namespace rendezllama {
namespace language {

struct SpecialToken {
  std::string alias;
  std::optional<std::string> name;
  std::vector<std::string> candidates;
};

struct Substitution {
  std::string protagonist_alias;
  std::string confidant_alias;
  std::string bos_token_alias;
  std::string eos_token_alias;
  std::vector<SpecialToken> special_tokens;
};

struct Language {
  Substitution substitution;
  rendezllama::inference::InferVia infer_via;
};

bool
populate_Substitution(
    Substitution& substitution,
    FildeshSxpb* sxpb,
    FildeshSxpbIT it);
bool
populate_Language(
    Language& language,
    FildeshSxpb* sxpb,
    FildeshSxpbIT it);

}  // namespace language

const FildeshSxprotoField* language_sxproto_schema();

}  // namespace rendezllama
#endif

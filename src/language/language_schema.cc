#include "src/language/language_schema.hh"

#include <cassert>
#include <fildesh/sxproto.h>

#include "src/language/inference_schema.hh"

static FildeshSxprotoField special_token_message[] = {
  {"alias", FILL_FildeshSxprotoField_STRING(1, INT_MAX)},
  {"name", FILL_DEFAULT_FildeshSxprotoField_ALIAS},
  {"candidates", FILL_DEFAULT_FildeshSxprotoField_STRINGS},
};
static FildeshSxprotoField substitution_message[] = {
  {"protagonist_alias", FILL_FildeshSxprotoField_STRING(1, INT_MAX)},
  {"confidant_alias", FILL_FildeshSxprotoField_STRING(1, INT_MAX)},
  {"bos_token_alias", FILL_FildeshSxprotoField_STRING(1, INT_MAX)},
  {"eos_token_alias", FILL_FildeshSxprotoField_STRING(1, INT_MAX)},
  {"special_tokens", FILL_FildeshSxprotoField_MESSAGES(special_token_message)},
};

const FildeshSxprotoField* rendezllama::language_sxproto_schema() {
  static FildeshSxprotoField toplevel_fields[] = {
    {"infer_via", FILL_DEFAULT_FildeshSxprotoField_ALIAS},
    {"substitution", FILL_FildeshSxprotoField_MESSAGE(substitution_message)},
  };
  DECLARE_TOPLEVEL_FildeshSxprotoField(schema, toplevel_fields);
  if (!schema->name) {
    FildeshSxprotoField tmp_field;
    tmp_field = *rendezllama::inference_sxproto_schema();
    tmp_field.name = toplevel_fields[0].name;
    tmp_field.tag_id = toplevel_fields[0].tag_id;
    toplevel_fields[0] = tmp_field;
    lone_toplevel_initialization_FildeshSxprotoField(schema);
  }
  return schema;
}

  bool
rendezllama::language::populate_Substitution(
    Substitution& substitution,
    FildeshSxpb* sxpb,
    FildeshSxpbIT it)
{
  if (nullish_FildeshSxpbIT(it)) {
    return true;
  }
  const char* tmp = NULL;
  if (lone_subfield_at_FildeshSxpb_to_str(&tmp, sxpb, it, "protagonist_alias")) {
    substitution.protagonist_alias = tmp;
  }
  if (lone_subfield_at_FildeshSxpb_to_str(&tmp, sxpb, it, "confidant_alias")) {
    substitution.confidant_alias = tmp;
  }
  if (lone_subfield_at_FildeshSxpb_to_str(&tmp, sxpb, it, "bos_token_alias")) {
    substitution.bos_token_alias = tmp;
  }
  if (lone_subfield_at_FildeshSxpb_to_str(&tmp, sxpb, it, "eos_token_alias")) {
    substitution.eos_token_alias = tmp;
  }
  FildeshSxpbIT special_it = lookup_subfield_at_FildeshSxpb(sxpb, it, "special_tokens");
  if (!nullish_FildeshSxpbIT(special_it)) {
    for (special_it = first_at_FildeshSxpb(sxpb, special_it); !nullish_FildeshSxpbIT(special_it);
         special_it = next_at_FildeshSxpb(sxpb, special_it)) {
      auto& special = substitution.special_tokens.emplace_back();
      if (lone_subfield_at_FildeshSxpb_to_str(&tmp, sxpb, special_it, "alias")) {
        special.alias = tmp;
      }
      assert(!special.alias.empty());
      FildeshSxpbIT candidate_it = lookup_subfield_at_FildeshSxpb(sxpb, special_it, "candidates");
      if (nullish_FildeshSxpbIT(candidate_it)) {
        special.candidates.push_back(special.alias);
      }
      else {
        for (candidate_it = first_at_FildeshSxpb(sxpb, candidate_it);
             !nullish_FildeshSxpbIT(candidate_it);
             candidate_it = next_at_FildeshSxpb(sxpb, candidate_it)) {
          special.candidates.push_back(str_value_at_FildeshSxpb(sxpb, candidate_it));
        }
      }
    }
  }
  return true;
}

  bool
rendezllama::language::populate_Language(
    Language& language,
    FildeshSxpb* sxpb,
    FildeshSxpbIT it)
{
  if (nullish_FildeshSxpbIT(it)) {
    return true;
  }
  FildeshSxpbIT sub_it;

  sub_it = lookup_subfield_at_FildeshSxpb(sxpb, it, "infer_via");
  rendezllama::inference::populate_InferVia(language.infer_via, sxpb, sub_it);

  sub_it = lookup_subfield_at_FildeshSxpb(sxpb, it, "substitution");
  if (!nullish_FildeshSxpbIT(sub_it)) {
    populate_Substitution(language.substitution, sxpb, sub_it);
  }
  return true;
}

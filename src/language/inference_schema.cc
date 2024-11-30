#include "src/language/inference_schema.hh"

#include <cstdio>

#include <fildesh/sxproto.h>

static FildeshSxprotoField penalize_with_fields[] = {
  {"token_count", FILL_FildeshSxprotoField_INT(1, INT_MAX)},
  {"repetition", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"frequency", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"presence", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
};

static FildeshSxprotoField xtc_fields[] = {
  {"probability", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"threshold", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
};

static FildeshSxprotoField dry_fields[] = {
  {"probability", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"threshold", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
};

static FildeshSxprotoField adjust_thru_manyof[] = {
  {"dry", FILL_FildeshSxprotoField_MESSAGE(dry_fields)},
  {"penalize_with", FILL_FildeshSxprotoField_MESSAGE(penalize_with_fields)},
  {"top_k", FILL_FildeshSxprotoField_INT(1, INT_MAX)},
  {"tfs_z", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"typical_p", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"top_p", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"min_p", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"temperature", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"xtc", FILL_FildeshSxprotoField_MESSAGE(xtc_fields)},
};

static FildeshSxprotoField mirostat_fields[] = {
  {"version", FILL_FildeshSxprotoField_INT(1, 2)},
  {"tau", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"eta", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
};

static FildeshSxprotoField pick_via_oneof[] = {
  {"mirostat", FILL_FildeshSxprotoField_MESSAGE(mirostat_fields)},
  {"probability", FILL_FildeshSxprotoField_MESSAGE(mirostat_fields)},
};

static FildeshSxprotoField sampling_fields[] = {
  {"adjust_thru", FILL_FildeshSxprotoField_MANYOF(adjust_thru_manyof)},
  {"pick_via", FILL_FildeshSxprotoField_LONEOF(pick_via_oneof)},
};

static FildeshSxprotoField infer_via_oneof[] = {
  {"sampling", FILL_FildeshSxprotoField_MESSAGE(sampling_fields)},
};

const FildeshSxprotoField* rendezllama::language_sxproto_schema() {
  static FildeshSxprotoField toplevel_fields[] = {
    {"infer_via", FILL_FildeshSxprotoField_LONEOF(infer_via_oneof)},
  };
  DECLARE_TOPLEVEL_FildeshSxprotoField(schema, toplevel_fields);
  lone_toplevel_initialization_FildeshSxprotoField(schema);
  return schema;
}

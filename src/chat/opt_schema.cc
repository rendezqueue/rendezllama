#include "opt_schema.hh"

#include <cstdio>

#include <fildesh/sxproto.h>

static FildeshSxprotoField chat_prefixes_m_message[] = {
  {"prefix", FILL_FildeshSxprotoField_STRING(1, INT_MAX)},
  {"suffix", FILL_FildeshSxprotoField_STRING(1, INT_MAX)},
};
static FildeshSxprotoField chat_prefixes_manyof[] = {
  {"", FILL_FildeshSxprotoField_STRING(1, INT_MAX)},
  {"m", FILL_FildeshSxprotoField_MESSAGE(chat_prefixes_m_message)},
};
static FildeshSxprotoField special_token_message[] = {
  {"name", FILL_FildeshSxprotoField_STRING(1, INT_MAX)},
};
static FildeshSxprotoField substitute_message[] = {
  {"protagonist_alias", FILL_FildeshSxprotoField_STRING(1, INT_MAX)},
  {"confidant_alias", FILL_FildeshSxprotoField_STRING(1, INT_MAX)},
  {"bos_token_alias", FILL_FildeshSxprotoField_STRING(1, INT_MAX)},
  {"eos_token_alias", FILL_FildeshSxprotoField_STRING(1, INT_MAX)},
  {"special_tokens", FILL_FildeshSxprotoField_MESSAGES(special_token_message)},
};

  const FildeshSxprotoField*
rendezllama::options_sxproto_schema()
{
  static FildeshSxprotoField toplevel_fields[] = {
    {"batch_count", FILL_FildeshSxprotoField_INT(1, INT_MAX)},
    {"chat_prefixes", FILL_FildeshSxprotoField_MANYOF(chat_prefixes_manyof)},
    {"confidant", FILL_FildeshSxprotoField_STRING(1, INT_MAX)},
    {"context_token_limit", FILL_FildeshSxprotoField_INT(1, INT_MAX)},
    {"coprocess_mode_on", FILL_DEFAULT_FildeshSxprotoField_BOOL},
    {"frequency_penalty", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
    {"linespace_on", FILL_DEFAULT_FildeshSxprotoField_BOOL},
    {"lora_base_model", FILL_FildeshSxprotoField_STRING(1, FILENAME_MAX)},
    {"lora_base", FILL_DEFAULT_FildeshSxprotoField_ALIAS},
    {"lora", FILL_FildeshSxprotoField_STRING(1, FILENAME_MAX)},
    {"min_p", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
    {"mirostat", FILL_FildeshSxprotoField_INT(0, 2)},
    {"mirostat_eta", FILL_FildeshSxprotoField_FLOAT(0, 10)},
    {"mirostat_tau", FILL_FildeshSxprotoField_FLOAT(0, 10)},
    {"mlock_on", FILL_DEFAULT_FildeshSxprotoField_BOOL},
    {"mmap_on", FILL_DEFAULT_FildeshSxprotoField_BOOL},
    {"model", FILL_FildeshSxprotoField_STRING(1, FILENAME_MAX)},
    {"model_token_limit", FILL_FildeshSxprotoField_INT(1, INT_MAX)},
    {"o_rolling", FILL_FildeshSxprotoField_STRING(1, FILENAME_MAX)},
    {"presence_penalty", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
    {"protagonist", FILL_FildeshSxprotoField_STRING(1, INT_MAX)},
    {"repeat_last_count", FILL_FildeshSxprotoField_INT(0, INT_MAX)},
    {"repeat_last_n", FILL_DEFAULT_FildeshSxprotoField_ALIAS},
    {"repeat_window", FILL_DEFAULT_FildeshSxprotoField_ALIAS},
    {"repeat_penalty", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
    {"seed", FILL_FildeshSxprotoField_INT(0, INT_MAX)},
    {"sentence_limit", FILL_FildeshSxprotoField_INT(0, INT_MAX)},
    {"sentence_terminals", FILL_DEFAULT_FildeshSxprotoField_STRINGS},
    {"sentence_token_limit", FILL_FildeshSxprotoField_INT(0, INT_MAX)},
    {"startspace_on", FILL_DEFAULT_FildeshSxprotoField_BOOL},
    {"substitute", FILL_FildeshSxprotoField_MESSAGE(substitute_message)},
    {"substitution", FILL_DEFAULT_FildeshSxprotoField_ALIAS},
    {"temperature", FILL_FildeshSxprotoField_FLOAT(0, 10)},
    {"temp", FILL_DEFAULT_FildeshSxprotoField_ALIAS},
    {"tfs_z", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
    {"thread_count", FILL_FildeshSxprotoField_INT(1, INT_MAX)},
    {"batch_thread_count", FILL_FildeshSxprotoField_INT(0, INT_MAX)},
    // {"sampling", FILL_FildeshSxprotoField_MESSAGE(sampling_message)},
    // {"infer", FILL_FildeshSxprotoField_MESSAGE(inference_message)},
    {"top_k", FILL_FildeshSxprotoField_INT(1, INT_MAX)},
    {"top_p", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
    {"typical_p", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
    {"x_answer", FILL_FildeshSxprotoField_STRING(1, FILENAME_MAX)},
    {"x_priming", FILL_FildeshSxprotoField_STRING(1, FILENAME_MAX)},
    {"x_rolling", FILL_FildeshSxprotoField_STRING(1, FILENAME_MAX)},
  };
  DECLARE_TOPLEVEL_FildeshSxprotoField(schema, toplevel_fields);
  lone_toplevel_initialization_FildeshSxprotoField(schema);
  return schema;
}

  const FildeshSxprotoField*
rendezllama::dynamic_options_sxproto_schema()
{
  static FildeshSxprotoField toplevel_fields[] = {
    {"chat_prefixes", FILL_FildeshSxprotoField_MANYOF(chat_prefixes_manyof)},
    {"confidant", FILL_FildeshSxprotoField_STRING(1, INT_MAX)},
    {"frequency_penalty", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
    {"min_p", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
    {"mirostat", FILL_FildeshSxprotoField_INT(0, 2)},
    {"mirostat_eta", FILL_FildeshSxprotoField_FLOAT(0, 10)},
    {"mirostat_tau", FILL_FildeshSxprotoField_FLOAT(0, 10)},
    {"presence_penalty", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
    {"protagonist", FILL_FildeshSxprotoField_STRING(1, INT_MAX)},
    {"repeat_last_count", FILL_FildeshSxprotoField_INT(0, INT_MAX)},
    {"repeat_last_n", FILL_DEFAULT_FildeshSxprotoField_ALIAS},
    {"repeat_window", FILL_DEFAULT_FildeshSxprotoField_ALIAS},
    {"repeat_penalty", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
    {"sentence_limit", FILL_FildeshSxprotoField_INT(0, INT_MAX)},
    {"sentence_terminals", FILL_DEFAULT_FildeshSxprotoField_STRINGS},
    {"sentence_token_limit", FILL_FildeshSxprotoField_INT(0, INT_MAX)},
    {"temperature", FILL_FildeshSxprotoField_FLOAT(0, 10)},
    {"temp", FILL_DEFAULT_FildeshSxprotoField_ALIAS},
    {"tfs_z", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
    {"thread_count", FILL_FildeshSxprotoField_INT(1, INT_MAX)},
    {"batch_thread_count", FILL_FildeshSxprotoField_INT(0, INT_MAX)},
    {"top_k", FILL_FildeshSxprotoField_INT(1, INT_MAX)},
    {"top_p", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
    {"typical_p", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  };
  DECLARE_TOPLEVEL_FildeshSxprotoField(schema, toplevel_fields);
  lone_toplevel_initialization_FildeshSxprotoField(schema);
  return schema;
}


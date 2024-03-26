
static FildeshSxprotoField sampling_adjustments_oneof[] = {
  {"repetition", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"frequency", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"presence", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"top_k", FILL_FildeshSxprotoField_INT(1, INT_MAX)},
  {"tfs_z", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"typical_p", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"top_p", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"min_p", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"temperature", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
};
static FildeshSxprotoField sampling_selection_oneof[] = {
  {"probabilistic", FILL_DEFAULT_FildeshSxprotoField_MESSAGE()},
  {"mirostat_v1", FILL_DEFAULT_FildeshSxprotoField_MESSAGE()},
  {"mirostat_v2", FILL_DEFAULT_FildeshSxprotoField_MESSAGE()},
  {"greedy", FILL_DEFAULT_FildeshSxprotoField_MESSAGE()},
};
static FildeshSxprotoField sampling_message[] = {
  {"penalty_token_count",
  {"adjustments", FILL_FildeshSxprotoField_MANYOF(sampling_adjustments_oneof)},
  {"pick_with", FILL_FildeshSxprotoField_ONEOF(sampling_selection_oneof)},
};

add_executable(example_prompt_parse_test
  "parse_test.cc"
)
target_link_libraries(example_prompt_parse_test PRIVATE
  chat_opt_cc
)
add_test(NAME example_prompt_assistant_alpaca_parse_test COMMAND example_prompt_parse_test
  "${PROJECT_SOURCE_DIR}/example/prompt/assistant_alpaca/setting.sxpb")
add_test(NAME example_prompt_assistant_chatml_parse_test COMMAND example_prompt_parse_test
  "${PROJECT_SOURCE_DIR}/example/prompt/assistant_chatml/setting.sxpb")
add_test(NAME example_prompt_assistant_coprocess_parse_test COMMAND example_prompt_parse_test
  "${PROJECT_SOURCE_DIR}/example/prompt/assistant_coprocess/setting.sxpb")
add_test(NAME example_prompt_assistant_gemma_parse_test COMMAND example_prompt_parse_test
  "${PROJECT_SOURCE_DIR}/example/prompt/assistant_gemma/setting.sxpb")
add_test(NAME example_prompt_assistant_llama_parse_test COMMAND example_prompt_parse_test
  "${PROJECT_SOURCE_DIR}/example/prompt/assistant_llama/setting.sxpb")
add_test(NAME example_prompt_assistant_mistral_parse_test COMMAND example_prompt_parse_test
  "${PROJECT_SOURCE_DIR}/example/prompt/assistant_mistral/setting.sxpb")
add_test(NAME example_prompt_assistant_plain_parse_test COMMAND example_prompt_parse_test
  "${PROJECT_SOURCE_DIR}/example/prompt/assistant_plain/setting.sxpb")
add_test(NAME example_prompt_assistant_vicuna_parse_test COMMAND example_prompt_parse_test
  "${PROJECT_SOURCE_DIR}/example/prompt/assistant_vicuna/setting.sxpb")
add_test(NAME example_prompt_confidant_alpaca_parse_test COMMAND example_prompt_parse_test
  "${PROJECT_SOURCE_DIR}/example/prompt/confidant_alpaca/setting.sxpb")
add_test(NAME example_prompt_roshambo_kira_parse_test COMMAND example_prompt_parse_test
  "${PROJECT_SOURCE_DIR}/example/prompt/roshambo_kira/setting.sxpb")

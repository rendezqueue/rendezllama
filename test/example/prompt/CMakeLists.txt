
add_executable(example_prompt_parse_test
  "parse_test.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/opt.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/opt.hh"
)
target_link_libraries(example_prompt_parse_test
  ${Fildesh_LIBRARIES}
)
add_test(NAME example_prompt_assistant_alpaca_parse_test COMMAND example_prompt_parse_test
  "${PROJECT_SOURCE_DIR}/example/prompt/assistant_alpaca/setting.sxproto")
add_test(NAME example_prompt_assistant_coprocess_parse_test COMMAND example_prompt_parse_test
  "${PROJECT_SOURCE_DIR}/example/prompt/assistant_coprocess/setting.sxproto")
add_test(NAME example_prompt_assistant_vicuna_parse_test COMMAND example_prompt_parse_test
  "${PROJECT_SOURCE_DIR}/example/prompt/assistant_vicuna/setting.sxproto")
add_test(NAME example_prompt_confidant_alpaca_parse_test COMMAND example_prompt_parse_test
  "${PROJECT_SOURCE_DIR}/example/prompt/confidant_alpaca/setting.sxproto")
add_test(NAME example_prompt_roshambo_kira_parse_test COMMAND example_prompt_parse_test
  "${PROJECT_SOURCE_DIR}/example/prompt/roshambo_kira/setting.sxproto")


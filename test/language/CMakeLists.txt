add_executable(language_inference_schema_test
  "inference_schema_test.cc"
)
target_link_libraries(language_inference_schema_test PRIVATE
  chat_opt_cc
)
add_test(NAME language_inference_schema_test COMMAND
  language_inference_schema_test
)

add_executable(language_schema_test
  "language_schema_test.cc"
)
target_link_libraries(language_schema_test PRIVATE
  chat_opt_cc
)
add_test(NAME language_schema_test COMMAND
  language_schema_test
)

add_executable(language_inference_test
  "inference_test.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/cmd.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/control.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/display.cc"
)
target_include_directories(language_inference_test PRIVATE
  ${LlamaCpp_INCLUDE_DIRS}
)
target_link_libraries(language_inference_test PRIVATE
  language_inference_cc
)
add_test(NAME language_inference_test COMMAND
  language_inference_test "${TinyStoriesGGUF_FILE}"
)

add_executable(language_vocabulary_test
  "vocabulary_test.cc"
)
target_include_directories(language_vocabulary_test PRIVATE
  ${LlamaCpp_INCLUDE_DIRS}
)
target_link_libraries(language_vocabulary_test PRIVATE
  language_vocabulary_cc
)
add_test(NAME language_vocabulary_test COMMAND
  language_vocabulary_test "${LlamaCpp_spm_vocab_FILE}"
)
add_test(NAME language_vocabulary_tinystories_test COMMAND
  language_vocabulary_test "${TinyStoriesGGUF_FILE}"
)

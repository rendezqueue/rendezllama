
add_executable(language_inference_schema_test
  "inference_schema_test.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/opt.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/opt.hh"
  "${PROJECT_SOURCE_DIR}/src/chat/opt_schema.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/opt_schema.hh"
  "${PROJECT_SOURCE_DIR}/src/language/inference_schema.cc"
  "${PROJECT_SOURCE_DIR}/src/language/inference_schema.hh"
)
target_link_libraries(language_inference_schema_test PRIVATE
  ${FildeshSxproto_LIBRARIES}
)
add_test(NAME language_inference_schema_test COMMAND
  language_inference_schema_test
)

add_executable(language_vocabulary_test
  "vocabulary_test.cc"
  "${PROJECT_SOURCE_DIR}/src/language/vocabulary.cc"
  "${PROJECT_SOURCE_DIR}/src/language/vocabulary.hh"
)
target_include_directories(language_vocabulary_test PRIVATE
  ${LlamaCpp_INCLUDE_DIRS}
)
target_link_libraries(language_vocabulary_test PRIVATE
  ${Fildesh_LIBRARIES}
  ${LlamaCpp_LIBRARIES}
)
add_test(NAME language_vocabulary_test COMMAND
  language_vocabulary_test "${LlamaCpp_VOCAB_MODEL}"
)

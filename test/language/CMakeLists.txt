
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

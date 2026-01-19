add_library(language_schema_cc
  "language_schema.cc"
  "language_schema.hh"
  "inference_schema.cc"
  "inference_schema.hh"
)
target_link_libraries(language_schema_cc PUBLIC
  ${FildeshSxproto_LIBRARIES}
)

add_library(language_vocabulary_cc
  "vocabulary.cc"
  "vocabulary.hh"
)
target_link_libraries(language_vocabulary_cc PUBLIC
  ${Fildesh_LIBRARIES}
  ${LlamaCpp_LIBRARIES}
)

add_library(language_inference_cc
  "inference.cc"
  "inference.hh"
)
target_link_libraries(language_inference_cc PUBLIC
  language_vocabulary_cc
  ${LlamaCpp_LIBRARIES}
)

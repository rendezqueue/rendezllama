add_library(language_schema_cc
  "language_schema.cc"
  "language_schema.hh"
  "inference_schema.cc"
  "inference_schema.hh"
)
target_link_libraries(language_schema_cc PRIVATE
  ${FildeshSxproto_LIBRARIES}
)

add_executable(tokenize
  "tokenize_main.cc"
  "tokenize.cc"
  "tokenize.hh"
  "${CMAKE_SOURCE_DIR}/src/language/vocabulary.cc"
  "${CMAKE_SOURCE_DIR}/src/language/vocabulary.hh"
)
target_link_libraries(tokenize PRIVATE
  ${Fildesh_LIBRARIES}
  ${LlamaCpp_LIBRARIES}
)

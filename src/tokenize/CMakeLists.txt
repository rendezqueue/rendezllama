add_executable(tokenize
  "tokenize_main.cc"
  "tokenize.cc"
  "tokenize.hh"
)
target_link_libraries(tokenize PRIVATE
  ${Fildesh_LIBRARIES}
  ${LlamaCpp_LIBRARIES}
)

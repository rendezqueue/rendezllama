include_directories("${LlamaCppIncludePath}")
add_executable(tokenize "tokenize.cc")
target_link_libraries(tokenize
  fildesh_lib
  llama_cpp_lib
  ${CMAKE_THREAD_LIBS_INIT}
)

include_directories("${LlamaCppIncludePath}")
add_executable(tokenize
  "tokenize_main.cc"
  "tokenize.cc"
  "tokenize.hh"
)
target_link_libraries(tokenize
  fildesh_lib
  llama_cpp_lib
  ${CMAKE_THREAD_LIBS_INIT}
)
if(LLAMA_OPENBLAS)
  target_link_options(tokenize PRIVATE ${BLAS_LINKER_FLAGS})
  target_link_libraries(tokenize ${BLAS_LIBRARIES})
endif()

include_directories("${LlamaCppIncludePath}")
add_executable(chat
  "chat_main.cc"
  "chat.cc"
  "chat.hh"
  "cmd.cc"
  "cmd.hh"
  "opt.cc"
  "opt.hh"
  "${CMAKE_SOURCE_DIR}/src/tokenize/tokenize.cc"
  "${CMAKE_SOURCE_DIR}/src/tokenize/tokenize.hh"
)
target_link_libraries(chat
  fildesh_lib
  llama_cpp_lib
  ${CMAKE_THREAD_LIBS_INIT}
)
if(LLAMA_OPENBLAS)
  target_link_options(chat PRIVATE ${BLAS_LINKER_FLAGS})
  target_link_libraries(chat ${BLAS_LIBRARIES})
endif()

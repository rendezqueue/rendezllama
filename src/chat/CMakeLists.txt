add_executable(chat
  "chat_main.cc"
  "chat.cc"
  "chat.hh"
  "cmd.cc"
  "cmd.hh"
  "display.cc"
  "display.hh"
  "opt.cc"
  "opt.hh"
  "trajectory.cc"
  "trajectory.hh"
  "${CMAKE_SOURCE_DIR}/src/tokenize/tokenize.cc"
  "${CMAKE_SOURCE_DIR}/src/tokenize/tokenize.hh"
)
target_link_libraries(chat
  ${Fildesh_LIBRARIES}
  ${LlamaCpp_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)
if(LLAMA_OPENBLAS)
  target_link_options(chat PRIVATE ${BLAS_LINKER_FLAGS})
  target_link_libraries(chat ${BLAS_LIBRARIES})
endif()

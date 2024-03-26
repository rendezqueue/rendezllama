add_executable(chat
  "chat_main.cc"
  "chat.cc"
  "cmd.cc"
  "cmd.hh"
  "display.cc"
  "display.hh"
  "guide.cc"
  "guide.hh"
  "opt.cc"
  "opt.hh"
  "opt_schema.cc"
  "opt_schema.hh"
  "trajectory.cc"
  "trajectory.hh"
  "${CMAKE_SOURCE_DIR}/src/language/inference.hh"
  "${CMAKE_SOURCE_DIR}/src/language/vocabulary.cc"
  "${CMAKE_SOURCE_DIR}/src/language/vocabulary.hh"
)
if (LLAMA_OPENBLAS_ON)
  target_compile_definitions(chat PRIVATE "LLAMA_OPENBLAS_ON=1")
endif()
target_link_libraries(chat PRIVATE
  ${FildeshSxproto_LIBRARIES}
  ${LlamaCpp_LIBRARIES}
)

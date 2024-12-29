
add_library(chat_opt_cc
  "opt.cc"
  "opt.hh"
  "opt_schema.cc"
  "opt_schema.hh"
)
target_link_libraries(chat_opt_cc PUBLIC
  language_schema_cc
)

add_executable(chat
  "chat_main.cc"
  "cmd.cc"
  "cmd.hh"
  "display.cc"
  "display.hh"
  "guide.cc"
  "guide.hh"
  "trajectory.cc"
  "trajectory.hh"
  "${CMAKE_SOURCE_DIR}/src/language/inference.cc"
  "${CMAKE_SOURCE_DIR}/src/language/inference.hh"
  "${CMAKE_SOURCE_DIR}/src/language/vocabulary.cc"
  "${CMAKE_SOURCE_DIR}/src/language/vocabulary.hh"
)
target_link_libraries(chat PRIVATE
  chat_opt_cc
  ${LlamaCpp_LIBRARIES}
)
if (LLAMA_OPENBLAS_ON)
  target_compile_definitions(chat PRIVATE "LLAMA_OPENBLAS_ON=1")
endif()

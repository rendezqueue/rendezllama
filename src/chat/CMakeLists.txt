
add_library(chat_opt_cc
  "opt.cc"
  "opt.hh"
  "opt_schema.cc"
  "opt_schema.hh"
)
target_link_libraries(chat_opt_cc PUBLIC
  language_schema_cc
)

add_library(chat_trajectory_cc
  "trajectory.cc"
  "trajectory.hh"
)
target_link_libraries(chat_trajectory_cc PUBLIC
  language_vocabulary_cc
  ${Fildesh_LIBRARIES}
  ${LlamaCpp_LIBRARIES}
)

add_executable(chat
  "chat_main.cc"
  "cmd.cc"
  "cmd.hh"
  "control.cc"
  "control.hh"
  "display.cc"
  "display.hh"
  "guide.cc"
  "guide.hh"
)
target_link_libraries(chat PRIVATE
  chat_opt_cc
  chat_trajectory_cc
  language_inference_cc
  ${LlamaCpp_LIBRARIES}
)
if (LLAMA_OPENBLAS_ON)
  target_compile_definitions(chat PRIVATE "LLAMA_OPENBLAS_ON=1")
endif()

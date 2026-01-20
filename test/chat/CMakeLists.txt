add_executable(chat_cmd_test
  "cmd_test.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/cmd.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/cmd.hh"
)
target_link_libraries(chat_cmd_test PRIVATE
  chat_opt_cc
  chat_trajectory_cc
)
add_test(NAME chat_cmd_test COMMAND
  chat_cmd_test "${LlamaCpp_spm_vocab_FILE}"
)

add_executable(chat_control_test
  "control_test.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/control.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/control.hh"
  "${PROJECT_SOURCE_DIR}/src/chat/display.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/display.hh"
)
target_link_libraries(chat_control_test PRIVATE
  chat_guide_cc
)
add_test(NAME chat_control_test COMMAND
  chat_control_test "${LlamaCpp_spm_vocab_FILE}"
)

add_executable(chat_display_test
  "display_test.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/display.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/display.hh"
)
target_link_libraries(chat_display_test PRIVATE
  chat_trajectory_cc
)
add_test(NAME chat_display_test COMMAND
  chat_display_test "${LlamaCpp_spm_vocab_FILE}"
)

add_executable(chat_guide_test
  "guide_test.cc"
)
target_link_libraries(chat_guide_test PRIVATE
  chat_guide_cc
)
add_test(NAME chat_guide_test COMMAND
  chat_guide_test "${LlamaCpp_spm_vocab_FILE}"
)

add_executable(chat_opt_test
  "opt_test.cc"
)
target_link_libraries(chat_opt_test PRIVATE
  chat_opt_cc
)
add_test(NAME chat_opt_test COMMAND
  chat_opt_test
)

add_executable(chat_trajectory_test
  "trajectory_test.cc"
)
target_link_libraries(chat_trajectory_test PRIVATE
  chat_trajectory_cc
)
add_test(NAME chat_trajectory_test COMMAND
  chat_trajectory_test "${LlamaCpp_spm_vocab_FILE}"
)

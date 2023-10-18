
add_executable(chat_opt_test
  "opt_test.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/opt.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/opt.hh"
)
target_link_libraries(chat_opt_test PRIVATE
  ${FildeshSxproto_LIBRARIES}
)
add_test(NAME chat_opt_test COMMAND
  chat_opt_test
)

add_executable(chat_trajectory_test
  "trajectory_test.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/trajectory.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/trajectory.hh"
  "${PROJECT_SOURCE_DIR}/src/language/vocabulary.cc"
  "${PROJECT_SOURCE_DIR}/src/language/vocabulary.hh"
)
target_link_libraries(chat_trajectory_test PRIVATE
  ${Fildesh_LIBRARIES}
  ${LlamaCpp_LIBRARIES}
)
add_test(NAME chat_trajectory_test COMMAND
  chat_trajectory_test "${LlamaCpp_VOCAB_MODEL}"
)

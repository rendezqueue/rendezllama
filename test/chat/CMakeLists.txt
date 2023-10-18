
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


add_executable(src_opt_test
  "opt_test.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/opt.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/opt.hh"
)
target_link_libraries(src_opt_test
  ${FildeshSxproto_LIBRARIES}
)
add_test(NAME src_opt_test COMMAND src_opt_test)


add_executable(src_size_test
  "size_test.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/trajectory.hh"
)
target_include_directories(src_size_test PRIVATE
  ${LlamaCpp_INCLUDE_DIRS}
)
add_test(NAME src_size_test COMMAND src_size_test)

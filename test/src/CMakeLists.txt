
add_executable(src_size_test
  "size_test.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/trajectory.hh"
)
target_include_directories(src_size_test PRIVATE
  ${LlamaCpp_INCLUDE_DIRS}
)
add_test(NAME src_size_test COMMAND src_size_test)

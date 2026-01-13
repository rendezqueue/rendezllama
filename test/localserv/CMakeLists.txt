find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_test(NAME localserv_test
  COMMAND Python3::Interpreter
  "${CMAKE_CURRENT_SOURCE_DIR}/localserv_test.py"
  "$<TARGET_FILE:localserv>"
)

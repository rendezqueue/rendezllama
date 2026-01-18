add_executable(assistant_cli assistant_cli/main.cc)
target_link_libraries(assistant_cli PRIVATE rendezllama_chat)

if(BUILD_TESTING)
  find_package(Python3 COMPONENTS Interpreter REQUIRED)
  add_test(
    NAME assistant_cli_test
    COMMAND Python3::Interpreter
            ${CMAKE_CURRENT_SOURCE_DIR}/assistant_cli/test_assistant_cli.py
            --binary $<TARGET_FILE:assistant_cli>
            --model "${LiteMistralGGUF_SOURCE_DIR}/lite-mistral-150m-v2-instruct-q4_k_m.gguf"
  )
endif()

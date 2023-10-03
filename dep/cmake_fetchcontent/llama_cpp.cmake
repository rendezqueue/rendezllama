FetchContent_Declare(
  LlamaCpp
  GIT_REPOSITORY "https://github.com/ggerganov/llama.cpp.git"
  GIT_TAG "40e07a60f9ce06e79f3ccd4c903eba300fb31b5e"
)
FetchContent_MakeAvailable(LlamaCpp)
set(LlamaCpp_INCLUDE_DIRS "${llamacpp_SOURCE_DIR}" PARENT_SCOPE)
set(LlamaCpp_LIBRARIES "$<TARGET_NAME:llama>" PARENT_SCOPE)

if(LLAMA_OPENBLAS)
  find_package(OpenBLAS REQUIRED)
  target_compile_definitions(ggml PRIVATE "GGML_USE_OPENBLAS")
  target_include_directories(ggml PRIVATE ${OpenBLAS_INCLUDE_DIRS})
  target_link_libraries(ggml PUBLIC ${OpenBLAS_LIBRARIES})
endif()

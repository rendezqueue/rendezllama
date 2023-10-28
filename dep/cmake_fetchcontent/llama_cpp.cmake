FetchContent_Declare(
  LlamaCpp
  GIT_REPOSITORY "https://github.com/ggerganov/llama.cpp.git"
  GIT_TAG "daab3d7f45832e10773c99f3484b0d5b14d86c0c"
)
FetchContent_MakeAvailable(LlamaCpp)
set(LlamaCpp_SOURCE_DIR "${llamacpp_SOURCE_DIR}" PARENT_SCOPE)
set(LlamaCpp_INCLUDE_DIRS "${llamacpp_SOURCE_DIR}" PARENT_SCOPE)
set(LlamaCpp_LIBRARIES "$<TARGET_NAME:llama>" PARENT_SCOPE)

if(LLAMA_OPENBLAS)
  find_package(OpenBLAS REQUIRED)
  target_compile_definitions(ggml PRIVATE "GGML_USE_OPENBLAS")
  target_include_directories(ggml PRIVATE ${OpenBLAS_INCLUDE_DIRS})
  target_link_libraries(ggml PUBLIC ${OpenBLAS_LIBRARIES})
endif()

FetchContent_Declare(
  LlamaCpp
  GIT_REPOSITORY "https://github.com/ggml-org/llama.cpp.git"
  GIT_TAG "d9c6ce46f747189cd6238ca7699253613f77c016"
)

FetchContent_MakeAvailable(LlamaCpp)

set(LlamaCpp_SOURCE_DIR "${llamacpp_SOURCE_DIR}" PARENT_SCOPE)
set(LlamaCpp_INCLUDE_DIRS "${llamacpp_SOURCE_DIR}/include" PARENT_SCOPE)
set(LlamaCpp_LIBRARIES "$<TARGET_NAME:llama>" PARENT_SCOPE)

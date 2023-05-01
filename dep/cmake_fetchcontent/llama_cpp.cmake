FetchContent_Declare(
  LlamaCpp
  GIT_REPOSITORY "https://github.com/ggerganov/llama.cpp.git"
  GIT_TAG "90b19bd6eee943832584f9cac0b6f9ea29cc42a4"
)
FetchContent_MakeAvailable(LlamaCpp)
set(LlamaCpp_INCLUDE_DIRS "${LlamaCpp_SOURCE_DIR}" PARENT_SCOPE)
set(LlamaCpp_LIBRARIES "$<TARGET_NAME:llama>" PARENT_SCOPE)

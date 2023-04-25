FetchContent_Declare(
  LlamaCpp
  GIT_REPOSITORY "https://github.com/ggerganov/llama.cpp.git"
  GIT_TAG "54bb60e26858be251a0eb3cb70f80322aff804a0"
)
FetchContent_MakeAvailable(LlamaCpp)
set(LlamaCpp_INCLUDE_DIRS "${LlamaCpp_SOURCE_DIR}" PARENT_SCOPE)
set(LlamaCpp_LIBRARIES "$<TARGET_NAME:llama>" PARENT_SCOPE)

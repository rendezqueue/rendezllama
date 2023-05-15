FetchContent_Declare(
  LlamaCpp
  GIT_REPOSITORY "https://github.com/ggerganov/llama.cpp.git"
  GIT_TAG "63d20469b85467c5729cc9a97bd44cc3da63423f"
)
FetchContent_MakeAvailable(LlamaCpp)
set(LlamaCpp_INCLUDE_DIRS "${llamacpp_SOURCE_DIR}" PARENT_SCOPE)
set(LlamaCpp_LIBRARIES "$<TARGET_NAME:llama>" PARENT_SCOPE)

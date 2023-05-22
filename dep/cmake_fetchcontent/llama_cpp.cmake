FetchContent_Declare(
  LlamaCpp
  GIT_REPOSITORY "https://github.com/ggerganov/llama.cpp.git"
  GIT_TAG "7e4ea5beff567f53be92f75f9089e6f11fa5dabd"
)
FetchContent_MakeAvailable(LlamaCpp)
set(LlamaCpp_INCLUDE_DIRS "${llamacpp_SOURCE_DIR}" PARENT_SCOPE)
set(LlamaCpp_LIBRARIES "$<TARGET_NAME:llama>" PARENT_SCOPE)

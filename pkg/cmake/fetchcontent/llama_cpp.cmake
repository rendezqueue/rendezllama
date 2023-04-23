FetchContent_Declare(
  llama_cpp
  GIT_REPOSITORY "https://github.com/ggerganov/llama.cpp.git"
  GIT_TAG "2f7c8e014e3c0ceaf39688845c2ff6f919fb03b7"
)
FetchContent_GetProperties(llama_cpp)
if (NOT llama_cpp_POPULATED)
  FetchContent_Populate(llama_cpp)
  add_subdirectory(${llama_cpp_SOURCE_DIR} ${llama_cpp_BINARY_DIR} EXCLUDE_FROM_ALL)
  set(LlamaCppIncludePath "${llama_cpp_SOURCE_DIR}")
  add_library(llama_cpp_lib ALIAS llama)
endif()


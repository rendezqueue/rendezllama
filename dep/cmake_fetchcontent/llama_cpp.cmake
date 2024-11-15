FetchContent_Declare(
  LlamaCpp
  GIT_REPOSITORY "https://github.com/ggerganov/llama.cpp.git"
  GIT_TAG "9f409893519b4a6def46ef80cd6f5d05ac0fb157"
)

set(GGML_OPENMP FALSE CACHE BOOL "OpenMP off for compatibility.")
FetchContent_MakeAvailable(LlamaCpp)

set(LlamaCpp_SOURCE_DIR "${llamacpp_SOURCE_DIR}" PARENT_SCOPE)
set(LlamaCpp_INCLUDE_DIRS "${llamacpp_SOURCE_DIR}/include" PARENT_SCOPE)
set(LlamaCpp_LIBRARIES "$<TARGET_NAME:llama>" PARENT_SCOPE)

if (LLAMA_OPENBLAS_ON)
  find_package(OpenBLAS REQUIRED)
  target_compile_definitions(ggml PRIVATE "GGML_USE_OPENBLAS")
  target_include_directories(ggml PRIVATE ${OpenBLAS_INCLUDE_DIRS})
  target_link_libraries(ggml PUBLIC ${OpenBLAS_LIBRARIES})
endif()

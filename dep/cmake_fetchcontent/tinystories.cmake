FetchContent_Declare(
  TinyStoriesGGUF
  GIT_REPOSITORY "https://huggingface.co/raincandy-u/TinyStories-656K-Q8_0-GGUF"
  GIT_TAG "d7b371e2fe29754622b522ea132de5f3aed00d7d"
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(TinyStoriesGGUF)

set(TinyStoriesGGUF_FILE "${tinystoriesgguf_SOURCE_DIR}/tinystories-656k-q8_0.gguf" PARENT_SCOPE)

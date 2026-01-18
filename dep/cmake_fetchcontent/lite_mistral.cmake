FetchContent_Declare(
  LiteMistralGGUF
  URL "https://huggingface.co/Alex01837178373/Lite-Mistral-150M-v2-Instruct-Q4_K_M-GGUF/resolve/main/lite-mistral-150m-v2-instruct-q4_k_m.gguf"
  DOWNLOAD_NO_EXTRACT TRUE
)

FetchContent_MakeAvailable(LiteMistralGGUF)

set(LiteMistralGGUF_SOURCE_DIR "${litemistralgguf_SOURCE_DIR}" PARENT_SCOPE)

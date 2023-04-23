
if(LLAMA_OPENBLAS)
  set(BLA_VENDOR OpenBLAS)
  find_package(BLAS REQUIRED)
endif()

find_package(Threads REQUIRED)

include(FetchContent)
include("cmake_fetchcontent/fildesh.cmake")
include("cmake_fetchcontent/llama_cpp.cmake")

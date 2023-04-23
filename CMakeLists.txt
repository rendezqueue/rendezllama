cmake_minimum_required(VERSION 3.12) # Don't bump this version for no reason

#set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
project("rendezllama" C CXX)
set(CMAKE_CXX_STANDARD 11)


option(LLAMA_OPENBLAS "llama: use OpenBLAS" OFF)


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(C_ARCH_FLAGS "")
if(CMAKE_BUILD_TYPE MATCHES "RelOnHost.*")
  if (APPLE)
    set(C_ARCH_FLAGS "-march=native")
    set(FILDESH_PREFER_POLL TRUE)
  elseif(UNIX)
    set(C_ARCH_FLAGS "-march=native")
    set(FILDESH_PREFER_AIO TRUE)
    set(FILDESH_BUILTIN_ELASTIC_POLL_ON TRUE)
  else()
    set(FILDESH_PREFER_PTHREAD TRUE)
  endif()
endif()


string(REPLACE "-DNDEBUG" "" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
string(REPLACE "/DNDEBUG" "" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
string(REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

set(CMAKE_C_FLAGS_RELONHOST "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_RELONHOST "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_RELONHOST "${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_RELONHOST "${CMAKE_CXX_FLAGS_RELEASE}")


include(CPack)
include(GNUInstallDirs)
include(ExternalProject)
include("pkg/cmake/externalproject/fildesh_project.cmake")
include(FetchContent)
include("pkg/cmake/fetchcontent/llama_cpp.cmake")


if(LLAMA_OPENBLAS)
  set(BLA_VENDOR OpenBLAS)
  find_package(BLAS REQUIRED)
endif()

find_package(Threads REQUIRED)


include_directories("${CMAKE_SOURCE_DIR}")
add_subdirectory(src)
add_subdirectory(example)


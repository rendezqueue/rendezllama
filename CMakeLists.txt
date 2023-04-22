cmake_minimum_required(VERSION 3.12) # Don't bump this version for no reason

#set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
project("rendezllama" C CXX)
set(CMAKE_CXX_STANDARD 11)


option(LLAMA_OPENBLAS "llama: use OpenBLAS" OFF)


enable_testing()

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


include(ExternalProject)
include(GNUInstallDirs)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/pkg/cmake/version.ini" version_info)


string(REGEX MATCH "fildesh_tgz_url=([^\n]*)" _ "${version_info}")
set(fildesh_tgz_url "${CMAKE_MATCH_1}")
string(REGEX MATCH "fildesh_tgz_hash=([^\n]*)" _ "${version_info}")
set(fildesh_tgz_hash "${CMAKE_MATCH_1}")
ExternalProject_Add(fildesh_project
  URL "${fildesh_tgz_url}"
  URL_HASH "${fildesh_tgz_hash}"
  CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>"
)
add_executable(fildesh IMPORTED)
add_library (fildesh_lib STATIC IMPORTED)
ExternalProject_Get_Property(fildesh_project INSTALL_DIR)
ExternalProject_Get_Property(fildesh_project INSTALL_DIR)
set_target_properties(fildesh PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/${CMAKE_INSTALL_BINDIR}/fildesh")
set_target_properties(fildesh_lib PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/fildesh/libfildesh.a")
include_directories("${INSTALL_DIR}/${CMAKE_INSTALL_INCLUDEDIR}")
add_dependencies(fildesh fildesh_project)
add_dependencies(fildesh_lib fildesh_project)


string(REGEX MATCH "llama_cpp_git_url=([^\n]*)" _ "${version_info}")
set(llama_cpp_git_url "${CMAKE_MATCH_1}")
string(REGEX MATCH "llama_cpp_git_tag=([^\n]*)" _ "${version_info}")
set(llama_cpp_git_tag "${CMAKE_MATCH_1}")
ExternalProject_Add(llama_cpp_project
  GIT_REPOSITORY "${llama_cpp_git_url}"
  # When updating this dependency, prefer commits that match a daily release.
  GIT_TAG "${llama_cpp_git_tag}"
  GIT_SHALLOW TRUE
  BUILD_BYPRODUCTS
  "<BINARY_DIR>/libllama.a"
  CMAKE_ARGS "-DLLAMA_OPENBLAS=${LLAMA_OPENBLAS}"
  INSTALL_COMMAND echo "No install step."
)
add_library (llama_cpp_lib STATIC IMPORTED)
ExternalProject_Get_Property(llama_cpp_project BINARY_DIR)
set_target_properties(llama_cpp_lib PROPERTIES IMPORTED_LOCATION "${BINARY_DIR}/libllama.a")
add_dependencies (llama_cpp_lib llama_cpp_project)
ExternalProject_Get_Property(llama_cpp_project SOURCE_DIR)
set(LlamaCppIncludePath "${SOURCE_DIR}")

if(LLAMA_OPENBLAS)
  set(BLA_VENDOR OpenBLAS)
  find_package(BLAS REQUIRED)
endif()

find_package(Threads REQUIRED)


include_directories("${CMAKE_SOURCE_DIR}")
add_subdirectory(src)
add_subdirectory(example)


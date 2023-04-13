cmake_minimum_required(VERSION 3.12) # Don't bump this version for no reason

#set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
project("rendezllama" C CXX)


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


ExternalProject_Add(fildesh_project
  URL "https://github.com/fildesh/fildesh/releases/download/v0.1.8/fildesh-0.1.8.tar.gz"
  URL_HASH "SHA256=09918bc72c57edd2934194c2df344b9afbbde414a87cbf30b73e53890ef4a249"
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


ExternalProject_Add(llama_cpp_project
  GIT_REPOSITORY "https://github.com/ggerganov/llama.cpp.git"
  GIT_TAG "180b693a47b6b825288ef9f2c39d24b6eea4eea6"
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


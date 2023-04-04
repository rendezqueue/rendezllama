cmake_minimum_required(VERSION 3.12) # Don't bump this version for no reason

#set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
project("rendezllama" C CXX)

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


ExternalProject_Add(fildesh_project
  URL "https://github.com/fildesh/fildesh/releases/download/v0.1.8/fildesh-0.1.8.tar.gz"
  URL_HASH "SHA256=09918bc72c57edd2934194c2df344b9afbbde414a87cbf30b73e53890ef4a249"
  CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>"
)
add_executable(fildesh IMPORTED)
add_library (fildesh_lib STATIC IMPORTED)
ExternalProject_Get_Property(fildesh_project INSTALL_DIR)
set_target_properties(fildesh PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/bin/fildesh")
set_target_properties(fildesh_lib PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/lib/fildesh/libfildesh.a")
include_directories("${INSTALL_DIR}/include")
add_dependencies(fildesh fildesh_project)
add_dependencies(fildesh_lib fildesh_project)


ExternalProject_Add(llama_cpp_project
  GIT_REPOSITORY "https://github.com/ggerganov/llama.cpp.git"
  GIT_TAG "53dbba769537e894ead5c6913ab2fd3a4658b738"
  BUILD_BYPRODUCTS
  "<BINARY_DIR>/libllama.a"
  INSTALL_COMMAND echo "No install step."
)
add_library (llama_cpp_lib STATIC IMPORTED)
ExternalProject_Get_Property(fildesh_project BINARY_DIR)
set_target_properties(llama_cpp_lib PROPERTIES IMPORTED_LOCATION "${BINARY_DIR}/libllama.a")
add_dependencies (llama_cpp_lib llama_cpp_project)


find_package(Threads REQUIRED)


include_directories("${CMAKE_SOURCE_DIR}")


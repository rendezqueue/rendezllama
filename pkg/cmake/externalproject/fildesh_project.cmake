ExternalProject_Add(fildesh_project
  GIT_REPOSITORY "https://github.com/fildesh/fildesh.git"
  GIT_TAG "dfeee4f3c0e8b1a74dec13e5a78bad5646db7917"
  CMAKE_ARGS
  "-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>"
  "-DBUILD_TESTING:BOOL=FALSE"
)
add_executable(fildesh IMPORTED)
add_library(fildesh_lib STATIC IMPORTED)
ExternalProject_Get_Property(fildesh_project INSTALL_DIR)
set_target_properties(fildesh PROPERTIES IMPORTED_LOCATION
  "${INSTALL_DIR}/${CMAKE_INSTALL_BINDIR}/fildesh"
)
set_target_properties(fildesh_lib PROPERTIES IMPORTED_LOCATION
  "${INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/fildesh/${CMAKE_STATIC_LIBRARY_PREFIX}fildesh${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
include_directories("${INSTALL_DIR}/${CMAKE_INSTALL_INCLUDEDIR}")
add_dependencies(fildesh fildesh_project)
add_dependencies(fildesh_lib fildesh_project)

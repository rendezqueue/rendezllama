FetchContent_Declare(
  Fildesh
  GIT_REPOSITORY "https://github.com/fildesh/fildesh.git"
  GIT_TAG "96a8a1e17d752e20414dccc4766e8cd1465835d3"
)
FetchContent_MakeAvailable(Fildesh)
set(Fildesh_INCLUDE_DIRS ${Fildesh_INCLUDE_DIRS} PARENT_SCOPE)
set(Fildesh_LIBRARIES ${Fildesh_LIBRARIES} PARENT_SCOPE)
set(FildeshSxproto_LIBRARIES ${FildeshSxproto_LIBRARIES} PARENT_SCOPE)

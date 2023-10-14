FetchContent_Declare(
  Fildesh
  GIT_REPOSITORY "https://github.com/fildesh/fildesh.git"
  GIT_TAG "d668e5ae3996d3ddd75e50ccca38c471a823334c"
)
FetchContent_MakeAvailable(Fildesh)
set(Fildesh_INCLUDE_DIRS ${Fildesh_INCLUDE_DIRS} PARENT_SCOPE)
set(Fildesh_LIBRARIES ${Fildesh_LIBRARIES} PARENT_SCOPE)
set(FildeshSxproto_LIBRARIES ${FildeshSxproto_LIBRARIES} PARENT_SCOPE)

FetchContent_Declare(
  Fildesh
  GIT_REPOSITORY "https://github.com/fildesh/fildesh.git"
  GIT_TAG "3d64380eff85034d6bf531f816453761f40e59fd"
)
FetchContent_MakeAvailable(Fildesh)
set(Fildesh_INCLUDE_DIRS ${Fildesh_INCLUDE_DIRS} PARENT_SCOPE)
set(Fildesh_LIBRARIES ${Fildesh_LIBRARIES} PARENT_SCOPE)
set(FildeshSxproto_LIBRARIES ${FildeshSxproto_LIBRARIES} PARENT_SCOPE)

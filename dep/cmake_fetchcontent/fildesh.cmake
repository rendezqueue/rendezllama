FetchContent_Declare(
  Fildesh
  GIT_REPOSITORY "https://github.com/fildesh/fildesh.git"
  GIT_TAG "9defdbec27700e01888d0834015e28947baba7cd"
)
FetchContent_MakeAvailable(Fildesh)
set(Fildesh_INCLUDE_DIRS ${Fildesh_INCLUDE_DIRS} PARENT_SCOPE)
set(Fildesh_LIBRARIES ${Fildesh_LIBRARIES} PARENT_SCOPE)
set(FildeshSxproto_LIBRARIES ${FildeshSxproto_LIBRARIES} PARENT_SCOPE)

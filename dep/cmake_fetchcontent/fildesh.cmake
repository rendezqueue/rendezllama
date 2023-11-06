FetchContent_Declare(
  Fildesh
  GIT_REPOSITORY "https://github.com/fildesh/fildesh.git"
  GIT_TAG "ea04f23b529dfda8ac17035dfdaea69602661907"
)
FetchContent_MakeAvailable(Fildesh)
set(Fildesh_INCLUDE_DIRS ${Fildesh_INCLUDE_DIRS} PARENT_SCOPE)
set(Fildesh_LIBRARIES ${Fildesh_LIBRARIES} PARENT_SCOPE)
set(FildeshSxproto_LIBRARIES ${FildeshSxproto_LIBRARIES} PARENT_SCOPE)

add_executable(localserv
  "localserv_main.cc"
)
target_link_libraries(localserv
  ${Fildesh_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)
if(WIN32)
  target_link_libraries(localserv wsock32 ws2_32)
endif()

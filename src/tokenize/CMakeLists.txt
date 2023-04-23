add_executable(tokenize
  "tokenize_main.cc"
  "tokenize.cc"
  "tokenize.hh"
)
target_include_directories(tokenize PRIVATE
  ${LlamaCpp_INCLUDE_DIRS}
)
target_link_libraries(tokenize
  ${Fildesh_LIBRARIES}
  ${LlamaCpp_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)
if(LLAMA_OPENBLAS)
  target_link_options(tokenize PRIVATE ${BLAS_LINKER_FLAGS})
  target_link_libraries(tokenize ${BLAS_LIBRARIES})
endif()

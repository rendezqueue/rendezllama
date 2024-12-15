
add_executable(eg_preset_parse_test
  "parse_test.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/opt.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/opt.hh"
  "${PROJECT_SOURCE_DIR}/src/chat/opt_schema.cc"
  "${PROJECT_SOURCE_DIR}/src/chat/opt_schema.hh"
  "${PROJECT_SOURCE_DIR}/src/language/inference_schema.cc"
  "${PROJECT_SOURCE_DIR}/src/language/inference_schema.hh"
)
target_link_libraries(eg_preset_parse_test
  ${FildeshSxproto_LIBRARIES}
)

file(GLOB_RECURSE preset_files "${PROJECT_SOURCE_DIR}/eg/preset/*.sxpb")
foreach(preset_file IN LISTS preset_files)
  get_filename_component(preset_name "${preset_file}" NAME_WE)
  string(REPLACE "." "_" preset_name "${preset_name}")
  add_test(
    NAME eg_preset_${preset_name}_parse_test
    COMMAND eg_preset_parse_test "${preset_file}"
  )
endforeach()

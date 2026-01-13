
add_executable(eg_preset_parse_test
  "parse_test.cc"
)
target_link_libraries(eg_preset_parse_test
  chat_opt_cc
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

# catalyst_embed_shaders(<target>)

# Generates EmbedShaders.inc from assets/shaders/**/*.glsl and attaches it
# to <target> as a generated source, so the build dependency is automatic.
function(catalyst_embed_shaders target)
  set(shader_dir "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders")
  set(gen_dir    "${CMAKE_CURRENT_BINARY_DIR}/generated")
  set(embed_inc  "${gen_dir}/EmbedShaders.inc")

  file(GLOB_RECURSE shader_files CONFIGURE_DEPENDS "${shader_dir}/*.glsl")

  add_custom_command(
    OUTPUT "${embed_inc}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${gen_dir}"
    COMMAND "${CMAKE_COMMAND}"
            "-DSHADER_DIR=${shader_dir}"
            "-DOUTPUT_FILE=${embed_inc}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShaders.cmake"
    DEPENDS ${shader_files}
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShaders.cmake"
    COMMENT "Embedding engine shaders"
    VERBATIM
  )

  target_sources(${target} PRIVATE "${embed_inc}")
  set_source_files_properties("${embed_inc}" PROPERTIES HEADER_FILE_ONLY TRUE)
  target_include_directories(${target} PRIVATE "${gen_dir}")
endfunction()

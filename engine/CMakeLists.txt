add_library(engine INTERFACE)

add_subdirectory(src/utils)
add_subdirectory(src/math)
add_subdirectory(src/core)
add_subdirectory(src/platform)
add_subdirectory(src/input)
add_subdirectory(src/ecs)
add_subdirectory(src/resources)
add_subdirectory(src/assets)
add_subdirectory(src/graphics)
# add_subdirectory(src/render)
add_subdirectory(src/components)
add_subdirectory(src/animation)
add_subdirectory(src/scene)
add_subdirectory(src/audio)
add_subdirectory(src/editor)

target_link_libraries(engine INTERFACE
  engine_utils
  engine_math
  engine_core
  engine_platform
  engine_input
  engine_ecs
  engine_resources
  engine_assets
  engine_graphics
  # engine_render
  engine_components
  engine_animation
  engine_scene
  engine_audio
  engine_editor
)

target_compile_definitions(engine INTERFACE
  $<$<CONFIG:Debug>:CATALYST_EDITOR>
  $<$<CONFIG:RelWithDebInfo>:CATALYST_EDITOR>
)

# Built-in shader embedding
set(CATALYST_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders")
set(CATALYST_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(CATALYST_EMBED_INC "${CATALYST_GEN_DIR}/EmbedShaders.inc")

file(GLOB_RECURSE CATALYST_SHADER_FILES CONFIGURE_DEPENDS
    "${CATALYST_SHADER_DIR}/*.glsl")

add_custom_command(
  OUTPUT "${CATALYST_EMBED_INC}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${CATALYST_GEN_DIR}"
  COMMAND "${CMAKE_COMMAND}"
          "-DSHADER_DIR=${CATALYST_SHADER_DIR}"
          "-DOUTPUT_FILE=${CATALYST_EMBED_INC}"
          -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShaders.cmake"
  DEPENDS ${CATALYST_SHADER_FILES}
          "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShaders.cmake"
  COMMENT "Embeddding engine shaders"
  VERBATIM
)

add_custom_target(catalyst_embed_shaders DEPENDS "${CATALYST_EMBED_INC}")
add_dependencies(engine_assets catalyst_embed_shaders)

target_include_directories(engine_assets PRIVATE "${CATALYST_GEN_DIR}")


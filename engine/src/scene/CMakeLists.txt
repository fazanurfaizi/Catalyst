add_library(engine_scene STATIC)

target_sources(engine_scene PRIVATE
  Scene.cpp
  SceneSerializer.cpp
)

target_include_directories(engine_scene PUBLIC ${CMAKE_SOURCE_DIR}/engine/include)

target_link_libraries(engine_scene
  PUBLIC
    engine_utils
    engine_math
    engine_ecs
    engine_components
    engine_animation
    engine_assets
    engine_core
    nlohmann_json::nlohmann_json
)

if(CATALYST_WARNING_FLAGS)
  target_compile_options(engine_scene PRIVATE ${CATALYST_WARNING_FLAGS})
endif()

add_library(engine_core STATIC)

target_sources(engine_core PRIVATE
  Game.cpp
  GameStateManager.cpp
  GameConfigSerializer.cpp
)

target_include_directories(engine_core PUBLIC ${CMAKE_SOURCE_DIR}/engine/include)

target_link_libraries(engine_core
  PUBLIC
    engine_utils
    engine_math
    engine_ecs
    engine_components
    engine_resources
    engine_assets
    engine_graphics
    engine_audio
    engine_input
    engine_platform
    nlohmann_json::nlohmann_json
    glad
    glfw
  PRIVATE
    imgui
)

if(CATALYST_WARNING_FLAGS)
  target_compile_options(engine_core PRIVATE ${CATALYST_WARNING_FLAGS})
endif()

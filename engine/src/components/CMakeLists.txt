add_library(engine_components INTERFACE)

target_include_directories(engine_components INTERFACE ${CMAKE_SOURCE_DIR}/engine/include)

target_link_libraries(engine_components
  INTERFACE
    engine_math
    engine_ecs
    nlohmann_json::nlohmann_json
)

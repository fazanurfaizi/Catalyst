add_library(engine_ecs INTERFACE)

target_include_directories(engine_ecs INTERFACE ${CMAKE_SOURCE_DIR}/engine/include)

target_link_libraries(engine_ecs
  INTERFACE
    engine_utils
    engine_math
    nlohmann_json::nlohmann_json
    field_reflection::field_reflection
)

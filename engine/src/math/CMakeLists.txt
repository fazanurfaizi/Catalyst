add_library(engine_math STATIC)

target_sources(engine_math PRIVATE Rect.cpp)

target_include_directories(engine_math PUBLIC ${CMAKE_SOURCE_DIR}/engine/include)

target_link_libraries(engine_math
  PUBLIC glm nlohmann_json::nlohmann_json
)

if(CATALYST_WARNING_FLAGS)
  target_compile_options(engine_math PRIVATE ${CATALYST_WARNING_FLAGS})
endif()

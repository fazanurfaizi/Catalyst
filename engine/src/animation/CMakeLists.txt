add_library(engine_animation STATIC)

target_sources(engine_animation PRIVATE AnimationLibrary.cpp)

target_include_directories(engine_animation PUBLIC ${CMAKE_SOURCE_DIR}/engine/include)

target_link_libraries(engine_animation
  PUBLIC engine_utils engine_math engine_components engine_resources
)

if(CATALYST_WARNING_FLAGS)
  target_compile_options(engine_animation PRIVATE ${CATALYST_WARNING_FLAGS})
endif()

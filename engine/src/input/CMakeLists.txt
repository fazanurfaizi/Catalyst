add_library(engine_input STATIC)

target_sources(engine_input PRIVATE InputSystem.cpp)

target_include_directories(engine_input PUBLIC ${CMAKE_SOURCE_DIR}/engine/include)

target_link_libraries(engine_input
  PUBLIC engine_utils engine_math engine_platform glfw
  PRIVATE glm
)

if(CATALYST_WARNING_FLAGS)
  target_compile_options(engine_input PRIVATE ${CATALYST_WARNING_FLAGS})
endif()

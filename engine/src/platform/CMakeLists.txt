add_library(engine_platform STATIC)

target_sources(engine_platform PRIVATE
  Display.cpp
  Window.cpp
)

target_include_directories(engine_platform PUBLIC ${CMAKE_SOURCE_DIR}/engine/include)

target_link_libraries(engine_platform
  PUBLIC engine_utils engine_math engine_input glfw glad
  PRIVATE glm
)

target_compile_definitions(engine_platform PUBLIC GLFW_INCLUDE_NONE=1)

if(CATALYST_WARNING_FLAGS)
  target_compile_options(engine_platform PRIVATE ${CATALYST_WARNING_FLAGS})
endif()

add_library(engine_editor STATIC)

target_sources(engine_editor PRIVATE AssetEditor.cpp)

target_include_directories(engine_editor PUBLIC ${CMAKE_SOURCE_DIR}/engine/include)

target_link_libraries(engine_editor
  PUBLIC  engine_utils engine_assets engine_graphics engine_core
  PRIVATE imgui
)

if(CATALYST_EDITOR)
  target_compile_definitions(engine_editor PRIVATE CATALYST_EDITOR=1)
endif()

if(CATALYST_WARNING_FLAGS)
  target_compile_options(engine_editor PRIVATE ${CATALYST_WARNING_FLAGS})
endif()

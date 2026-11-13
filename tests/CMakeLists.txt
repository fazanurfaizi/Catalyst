add_executable(engine_tests
  test_main.cpp
  asset/test_atomic_file_writer.cpp
  asset/test_registry_serializer.cpp
  asset/test_asset_manager_editor.cpp
  asset/test_audio_engine.cpp
  core/test_game_config_serializer.cpp
)

target_link_libraries(engine_tests PRIVATE
  engine
  nlohmann_json::nlohmann_json
)

target_include_directories(engine_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(GAME_WARNING_FLAGS)
  target_compile_options(engine_tests PRIVATE ${GAME_WARNING_FLAGS})
endif()

add_test(NAME engine_tests COMMAND engine_tests)

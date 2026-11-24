add_library(engine_audio STATIC)

target_sources(engine_audio PRIVATE
  AudioEngine.cpp
  MusicPlayer.cpp
)

target_include_directories(engine_audio PUBLIC ${CMAKE_SOURCE_DIR}/engine/include)

target_link_libraries(engine_audio
  PUBLIC  engine_utils engine_resources
  PRIVATE miniaudio
)

if(CATALYST_WARNING_FLAGS)
  target_compile_options(engine_audio PRIVATE ${CATALYST_WARNING_FLAGS})
endif()

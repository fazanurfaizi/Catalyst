add_library(game STATIC
  src/game/states/PlayingState.cpp
)

target_include_directories(game PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(game PUBLIC
  engine
  imgui
)

if(CATALYST_PRODUCTION_BUILD)
  target_compile_definitions(game PRIVATE ASSETS_PATH="./assets/")
else()
  target_compile_definitions(game PRIVATE ASSETS_PATH="${CMAKE_SOURCE_DIR}/assets/")
endif()

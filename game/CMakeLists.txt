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

if(GAME_PRODUCTION_BUILD)
  target_compile_definitions(game PRIVATE ASSETS_PATH="./assets/")
else()
  # CMAKE_SOURCE_DIR points to the root KingdomRushFrontiers folder
  target_compile_definitions(game PRIVATE ASSETS_PATH="${CMAKE_SOURCE_DIR}/assets/")
endif()

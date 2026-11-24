add_library(engine_render STATIC)

target_sources(engine_render PRIVATE
  Renderer.cpp
  RenderGraph.cpp
  RenderDataList.cpp
  RenderTarget.cpp
  FullscreenQuad.cpp
  Camera2D.cpp

  sprite/SpriteBatch.cpp
  sprite/SpriteDrawList.cpp

  text/Font.cpp
  text/GlyphAtlas.cpp
  text/TextLayout.cpp
  text/TextRenderer.cpp

  lighting/LightRenderer.cpp
  lighting/LightTileCuller.cpp

  picking/IdPass.cpp

  debug/DebugRenderer.cpp

  passes/BackgroundPass.cpp
  passes/GameplayPass.cpp
  passes/LightingPass.cpp
  passes/UIPass.cpp
  passes/PostProcessPass.cpp
)

target_include_directories(engine_render PUBLIC ${CMAKE_SOURCE_DIR}/engine/include)

target_link_libraries(engine_render
  PUBLIC  engine_engine engine_math engine_core
  PRIVATE glm
)

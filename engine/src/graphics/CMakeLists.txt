add_library(engine_graphics STATIC)

target_include_directories(engine_graphics PUBLIC ${CMAKE_SOURCE_DIR}/engine/include)

target_sources(engine_graphics PRIVATE
  gl/Shader.cpp

  lighting/LightRenderer.cpp
  lighting/LightTileCuller.cpp

  pipeline/FullscreenQuad.cpp
  pipeline/RenderTarget.cpp

  sprite/SpriteBatch.cpp
  sprite/SpriteDrawList.cpp

  text/Font.cpp
  text/TextLayout.cpp

  texture/Texture.cpp

  Renderer.cpp
  TextRenderer.cpp
)

target_link_libraries(engine_graphics
  PUBLIC
    engine_utils
    engine_math
    engine_components
    engine_resources
    engine_assets
    engine_scene
    glad
  PRIVATE
    glm
)

if(CATALYST_WARNING_FLAGS)
  target_compile_options(engine_graphics PRIVATE ${CATALYST_WARNING_FLAGS})
endif()

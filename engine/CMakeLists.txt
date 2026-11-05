add_library(engine STATIC
  src/animation/AnimationLibrary.cpp

  src/assets/AssetManager.cpp

  src/audio/AudioEngine.cpp

  src/core/Game.cpp
  src/core/GameStateManager.cpp

  src/graphics/Renderer.cpp
  src/graphics/TextRenderer.cpp

  src/graphics/gl/Shader.cpp

  src/graphics/lighting/LightRenderer.cpp
  src/graphics/lighting/LightTileCuller.cpp

  src/graphics/pipeline/FullscreenQuad.cpp
  src/graphics/pipeline/RenderTarget.cpp
  
  src/graphics/sprite/SpriteBatch.cpp
  src/graphics/sprite/SpriteDrawList.cpp

  src/graphics/stb/stbi_image_impl.cpp
  src/graphics/stb/stb_truetype_impl.cpp

  src/graphics/text/Font.cpp
  src/graphics/text/TextLayout.cpp

  src/graphics/texture/Texture.cpp
  src/graphics/texture/TextureAtlas.cpp
  
  src/input/InputSystem.cpp
  
  src/math/Rect.cpp

  src/platform/Display.cpp
  src/platform/Window.cpp

  src/scene/World.cpp

  src/utils/Logger.cpp
)

# Public include root. Consumers see: include <glgame/core/Logger.hpp>
target_include_directories(engine PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# C++20 propagates to consumers automatically
target_compile_features(engine PUBLIC cxx_std_20)

# Vendored include dirs the engine uses internally (not part of the public API)
target_include_directories(engine PRIVATE
  "${CMAKE_SOURCE_DIR}/vendor/imgui"
  "${CMAKE_SOURCE_DIR}/vendor/stb-image/include"
)

# Public API leak-through: game code will see glm/glfw/glad through engine.
target_link_libraries(engine PUBLIC
  glm
  glfw
  glad
)

# Internal-only dependencies.
target_link_libraries(engine PRIVATE
  imgui
  stb_image
  nlohmann_json::nlohmann_json
  miniaudio
)

target_compile_definitions(engine PUBLIC
  GLFW_INCLUDE_NONE=1
)

# Propagate warnings from the root CMakeLists (see previous answer)
if(GAME_WARNING_FLAGS)
  target_compile_options(engine PRIVATE ${GAME_WARNING_FLAGS})
endif()


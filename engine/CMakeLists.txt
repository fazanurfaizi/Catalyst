add_library(engine STATIC
  src/core/Logger.cpp
  src/core/Game.cpp
  src/core/GameStateManager.cpp
  src/graphics/Camera.cpp
  src/scene/Scene.cpp
  src/graphics/Shader.cpp
  src/graphics/SpriteBatch.cpp
  src/graphics/Texture.cpp
  src/graphics/TextureAtlas.cpp
  src/graphics/Renderer.cpp
  src/graphics/stbi_image_impl.cpp
  src/animation/AnimationLibrary.cpp
  src/scene/SceneSerializer.cpp
  src/animation/ParticleSystem.cpp
)

# Public include root. Consumers see: include <glgame/core/Logger.hpp>
target_include_directories(engine PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# C++17 propagates to consumers automatically
target_compile_features(engine PUBLIC cxx_std_17)

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
)

target_compile_definitions(engine PUBLIC
  GLFW_INCLUDE_NONE=1
)

# Propagate warnings from the root CMakeLists (see previous answer)
if(GAME_WARNING_FLAGS)
  target_compile_options(engine PRIVATE ${GAME_WARNING_FLAGS})
endif()

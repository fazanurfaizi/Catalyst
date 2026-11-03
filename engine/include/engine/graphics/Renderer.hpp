#pragma once

#include "engine/graphics/FullscreenQuad.hpp"
#include "engine/graphics/LightBuffer.hpp"
#include "engine/graphics/LightRenderer.hpp"
#include "engine/graphics/LightTileCuller.hpp"
#include "engine/graphics/RenderTarget.hpp"
#include "engine/graphics/Shader.hpp"
#include "engine/graphics/SpriteDrawList.hpp"
#include "engine/graphics/Texture.hpp"
#include "engine/math/Color.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>

namespace engine::graphics {

class SpriteBatch;

class Scene;

class Renderer {
public:
  explicit Renderer();
  ~Renderer();

  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

  /* Called once after GL context creation. Sets up offscreen targets and
   * loads the deferred-lighting / composite shaders. Safe to call multiple
   * times; only reallocates when the size actually changes.
   */
  void Init(int width, int height);

  /* Called from window-resize events. */
  void Resize(int width, int height);

  void Begin(const glm::mat4 &viewProjection);
  void End();

  void Draw(const glm::vec2 &position, const glm::vec2 &scale, float rotation,
            const math::Color color, const Texture *texture,
            const glm::vec2 &uvMin = {0, 0}, const glm::vec2 &uvMax = {1, 1},
            int layer = 0);

  std::uint32_t DrawCalls() const;
  std::uint32_t QuadCount() const;

  void SetAmbientLight(const glm::vec3 &color) { m_ambientColor = color; }

  LightBuffer &MutableLightBuffer() { return m_lightBuffer; }

private:
  void EnsureRenderTargets();
  void SetTileCounts(int width, int height);
  void UpdateTileGrid(const glm::mat4 &viewProjection);
  void ApplyLighting(const glm::mat4 &viewProjection);
  void Composite();
  void BuildTileGrid();

  std::unique_ptr<SpriteBatch> m_batch;
  SpriteDrawList m_drawList;

  RenderTarget m_sceneRT;
  RenderTarget m_lightRT;

  Shader m_lightShader;
  Shader m_compositeShader;
  FullscreenQuad m_fullscreenQuad;

  LightRenderer m_lightRenderer;
  LightBuffer m_lightBuffer;
  LightTileCuller m_tileCuller;

  glm::mat4 m_viewProjection{1.0f};
  glm::vec3 m_ambientColor{0.05f};

  int m_width = 0, m_height = 0;
  int m_prevViewport[4] = {0, 0, 0, 0};

  bool m_usingDeferred = false;
};

} // namespace engine::graphics

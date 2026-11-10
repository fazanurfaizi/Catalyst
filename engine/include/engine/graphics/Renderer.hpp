#pragma once

#include "engine/asset/manager/IAssetManager.hpp"
#include "engine/graphics/gl/Shader.hpp"
#include "engine/graphics/lighting/LightBuffer.hpp"
#include "engine/graphics/lighting/LightRenderer.hpp"
#include "engine/graphics/lighting/LightTileCuller.hpp"
#include "engine/graphics/pipeline/FullscreenQuad.hpp"
#include "engine/graphics/pipeline/RenderPipeline.hpp"
#include "engine/graphics/pipeline/RenderStage.hpp"
#include "engine/graphics/pipeline/RenderTarget.hpp"
#include "engine/graphics/sprite/SpriteDrawList.hpp"
#include "engine/graphics/texture/Texture.hpp"
#include "engine/math/Color.hpp"
#include "engine/math/Rect.hpp"
#include "engine/scene/World.hpp"

#include <cstdint>
#include <memory>

namespace engine::graphics {

class SpriteBatch;

class Renderer {
public:
  explicit Renderer();
  ~Renderer();

  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

  void Init(int width, int height);
  void Resize(int width, int height);

  [[nodiscard]] float AspectRatio() const;
  [[nodiscard]] glm::mat4 UiProjection() const;
  [[nodiscard]] int Width() const { return m_width; }
  [[nodiscard]] int Height() const { return m_height; }

  void BeginFrame(const glm::mat4 &viewProjection,
                  const math::Rect &visibleBounds);
  void EndFrame();

  void RenderSprites(scene::World &world, asset::IAssetManager *assets);
  // void RenderParticles(scene::World &world, asset::IAssetManager *assets);
  void RenderLights(scene::World &world);

  void Draw(const glm::vec2 &position, const glm::vec2 &scale, float rotation,
            const math::Color color, const Texture *texture,
            const glm::vec2 &uvMin = {0, 0}, const glm::vec2 &uvMax = {1, 1},
            int layer = 0);

  [[nodiscard]] bool BeginStage(RenderStage s) {
    return m_pipeline.BeginStage(s);
  }
  void EndStage(RenderStage s) { m_pipeline.EndStage(s); }
  void BlitSource(Attachment src) { m_blitOverride = src; }

  void SetAmbientLight(const glm::vec3 &color) { m_ambientColor = color; }
  LightBuffer &MutableLightBuffer() { return m_lightBuffer; }

  [[nodiscard]] std::uint32_t DrawCalls() const { return m_lastDrawCalls; }
  [[nodiscard]] std::uint32_t QuadCount() const { return m_lastQuadCount; }

private:
  void EnsureRenderTargets();
  void UpdateTileGrid(const glm::mat4 &viewProjection);
  void BuildTileGrid();
  void SetTileCounts(int width, int height);

  void ExecuteSprite();
  void ExecuteLight();
  void ExecuteComposite();
  void ExecuteBlit(Attachment src);
  void ExecuteClearDepth();

  std::unique_ptr<SpriteBatch> m_batch;
  SpriteDrawList m_drawList;
  RenderPipeline m_pipeline;

  RenderTarget m_sceneRT;
  RenderTarget m_lightRT;

  Shader m_lightShader;
  Shader m_compositeShader;
  FullscreenQuad m_fullscreenQuad;

  LightRenderer m_lightRenderer;
  LightBuffer m_lightBuffer;
  LightTileCuller m_tileCuller;

  std::vector<SpriteInstance> m_instanceScratch;

  glm::mat4 m_viewProjection{1.0f};
  math::Rect m_visibleBounds{};
  glm::vec3 m_ambientColor{0.05f};

  int m_width = 0, m_height = 0;
  bool m_frameActive = false;
  bool m_hasSprites = false;
  bool m_hasLights = false;
  Attachment m_blitOverride = AttachNone;

  std::uint32_t m_lastDrawCalls = 0;
  std::uint32_t m_lastQuadCount = 0;
};

} // namespace engine::graphics

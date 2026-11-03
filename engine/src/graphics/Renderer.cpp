#include "engine/graphics/Renderer.hpp"

#include "engine/assets/AssetPath.hpp"
#include "engine/graphics/SpriteBatch.hpp"
#include "engine/graphics/SpriteDrawList.hpp"
#include "engine/utils/Logger.hpp"
#include "glad/gl.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <vector>

namespace engine::graphics {

Renderer::Renderer()
    : m_batch(std::make_unique<SpriteBatch>()) {}

Renderer::~Renderer() = default;

void Renderer::Init(int width, int height) {
  if (width <= 0 || height <= 0) {
    Logger::Error("[Renderer::Init] invalid size %dx%d", width, height);
    return;
  }

  m_sceneRT.Create(width, height);

  m_lightRT.Create(std::max(1, width / 2), std::max(1, height / 2));

  if (!m_lightShader.IsValid()) {
    if (!m_lightShader.LoadFromFile(AssetPath("shaders/light"))) {
      Logger::Error("[Renderer] failed to load 'shaders/light' - "
                    "lighting pipeline disabled (scene renders unlit).");
    }
  }

  if (!m_compositeShader.IsValid()) {
    if (!m_compositeShader.LoadFromFile(AssetPath("shaders/composite"))) {
      Logger::Error("[Renderer] failed to load 'shaders/composite' - "
                    "lighting pipeline disabled (scene renders unlit).");
    }
  }

  SetTileCounts(width, height);
}

void Renderer::SetTileCounts(int width, int height) {
  // Tile counts come from the framebuffer, but the world-space size of a tile
  // depends on the camera, so it is recomputed every frame in UpdateTileGrid.
  const auto tilesX = static_cast<std::uint32_t>(
      std::max(1, static_cast<int>(std::ceil(
                      static_cast<float>(width) /
                      static_cast<float>(LightTileCuller::kTileSizePx)))));
  const auto tilesY = static_cast<std::uint32_t>(
      std::max(1, static_cast<int>(std::ceil(
                      static_cast<float>(height) /
                      static_cast<float>(LightTileCuller::kTileSizePx)))));
  m_tileCuller.SetTileGrid(tilesX, tilesY, 1.0f, 1.0f, glm::vec2(0.0f));
}

void Renderer::UpdateTileGrid(const glm::mat4 &viewProjection) {
  const glm::mat4 invVP = glm::inverse(viewProjection);

  auto unproject = [&invVP](float x, float y) {
    glm::vec4 p = invVP * glm::vec4(x, y, 0.0f, 1.0f);
    const float invW = (std::abs(p.w) > 1e-6f) ? 1.0f / p.w : 1.0f;
    return glm::vec2(p.x * invW, p.y * invW);
  };

  const glm::vec2 c0 = unproject(-1.0f, -1.0f);
  const glm::vec2 c1 = unproject(1.0f, -1.0f);
  const glm::vec2 c2 = unproject(1.0f, 1.0f);
  const glm::vec2 c3 = unproject(-1.0f, 1.0f);

  const glm::vec2 lo = glm::min(glm::min(c0, c1), glm::min(c2, c3));
  const glm::vec2 hi = glm::max(glm::max(c0, c1), glm::max(c2, c3));

  // Guard against a degenerate/identity projection.
  const float worldW = std::max(hi.x - lo.x, 1e-3f);
  const float worldH = std::max(hi.y - lo.y, 1e-3f);

  const auto tilesX = m_tileCuller.TilesX();
  const auto tilesY = m_tileCuller.TilesY();

  m_tileCuller.SetTileGrid(tilesX, tilesY,
                           worldW / static_cast<float>(tilesX),
                           worldH / static_cast<float>(tilesY), lo);
}

void Renderer::Resize(int width, int height) {
  if (width <= 0 || height <= 0)
    return;
  m_width = width;
  m_height = height;
  m_sceneRT.Resize(width, height);
  m_lightRT.Resize(std::max(1, width / 2), std::max(1, height / 2));
  SetTileCounts(width, height);
}

void Renderer::EnsureRenderTargets() {
  if (m_sceneRT.ColorTexture() != 0 && m_lightRT.ColorTexture() != 0)
    return;
  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);
  if (vp[2] > 0 && vp[3] > 0)
    Init(vp[2], vp[3]);
}

void Renderer::Begin(const glm::mat4 &viewProjection) {
  EnsureRenderTargets();
  m_viewProjection = viewProjection;
  m_batch->Begin(m_viewProjection);
  m_drawList.Clear();

  const bool ready = m_lightShader.IsValid() && m_compositeShader.IsValid() &&
                     m_sceneRT.ColorTexture() != 0 &&
                     m_lightRT.ColorTexture() != 0;

  m_usingDeferred = ready;

  if (!ready) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glGetIntegerv(GL_VIEWPORT, m_prevViewport);
    glClearColor(0.05f, 0.05f, 0.07f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    return;
  }

  glGetIntegerv(GL_VIEWPORT, m_prevViewport);

  m_sceneRT.Bind(0, 0, m_sceneRT.Width(), m_sceneRT.Height());
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::Draw(const glm::vec2 &position, const glm::vec2 &scale,
                    float rotation, const math::Color color,
                    const Texture *texture, const glm::vec2 &uvMin,
                    const glm::vec2 &uvMax, int layer) {
  // m_batch->DrawQuad(position, scale, rotation, color, texture, uvMin, uvMax);
  m_drawList.Push(position, scale, rotation, color, texture, uvMin, uvMax);
}

void Renderer::End() {
  m_drawList.BuildBatches();
  for (const auto &group : m_drawList.Groups()) {
    const auto &cmds = m_drawList.Commands();

    static_assert(offsetof(SpriteCommand, instance) == 0,
                  "SpriteCommand must start with SpriteInstance");

    const auto *src =
        reinterpret_cast<const SpriteInstance *>(&cmds[group.start]);

    static thread_local std::vector<SpriteInstance> scratch;
    scratch.resize(group.count);

    for (std::size_t i = 0; i < group.count; ++i) {
      scratch[i] = cmds[group.start + i].instance;
    }
    m_batch->DrawInstances(scratch.data(), scratch.size(), group.texture);
  }
  m_batch->End();

  if (!m_usingDeferred)
    return;

  ApplyLighting(m_viewProjection);
  Composite();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(m_prevViewport[0], m_prevViewport[1], m_prevViewport[2],
             m_prevViewport[3]);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

std::uint32_t Renderer::DrawCalls() const { return m_batch->LastDrawCalls(); }
std::uint32_t Renderer::QuadCount() const {
  return m_batch->LastInstanceCount();
}

void Renderer::BuildTileGrid() {
  m_tileCuller.Build(m_lightBuffer.points, m_lightBuffer.pointCount);
  m_lightRenderer.UploadTileGrid(
      m_tileCuller.Offsets().data(), m_tileCuller.Offsets().size(),
      m_tileCuller.Indices().data(), m_tileCuller.Indices().size());
}

void Renderer::ApplyLighting(const glm::mat4 &viewProjection) {
  UpdateTileGrid(viewProjection);
  BuildTileGrid();
  m_lightRenderer.Upload(m_lightBuffer);

  m_lightRT.Bind(0, 0, m_lightRT.Width(), m_lightRT.Height());
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_BLEND);

  m_lightShader.Bind();
  m_lightShader.SetInt("uPointLightCount", m_lightBuffer.pointCount);
  m_lightShader.SetInt("uDirectionalLightCount",
                       m_lightBuffer.directionalCount);
  m_lightShader.SetInt("uSpotLightCount", m_lightBuffer.spotCount);
  m_lightShader.SetVec3("uAmbientColor", m_ambientColor);

  // Inverse VP -> shader reconstructs world position from NDC
  const glm::mat4 invVP = glm::inverse(viewProjection);
  m_lightShader.SetMat4("uInViewProjection", invVP);
  // uTileCountX/Y are `uint` in the shader: glUniform1i is invalid for them
  // and would leave both at 0, which breaks the tile lookup entirely.
  m_lightShader.SetUInt("uTileCountX", m_tileCuller.TilesX());
  m_lightShader.SetUInt("uTileCountY", m_tileCuller.TilesY());
  m_lightShader.SetVec2("uTileWorldSize",
                        {m_tileCuller.TileWorldW(), m_tileCuller.TileWorldH()});
  m_lightShader.SetVec2("uWorldOrigin", {m_tileCuller.WorldOrigin().x,
                                         m_tileCuller.WorldOrigin().y});

  m_fullscreenQuad.Draw();

  glEnable(GL_BLEND);
}

void Renderer::Composite() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(m_prevViewport[0], m_prevViewport[1], m_prevViewport[2],
             m_prevViewport[3]);
  glDisable(GL_BLEND);

  m_compositeShader.Bind();
  m_compositeShader.SetInt("uScene", 0);
  m_compositeShader.SetInt("uLight", 1);
  m_sceneRT.BindColorTexture(0);
  m_lightRT.BindColorTexture(1);

  m_fullscreenQuad.Draw();

  glActiveTexture(GL_TEXTURE0);

  glEnable(GL_BLEND);
}

} // namespace engine::graphics

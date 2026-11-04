#include "engine/graphics/Renderer.hpp"

#include "engine/assets/AssetPath.hpp"
#include "engine/components/Light.hpp"
#include "engine/components/Sprite.hpp"
#include "engine/components/Transform.hpp"
#include "engine/graphics/sprite/SpriteBatch.hpp"
#include "engine/graphics/sprite/SpriteDrawList.hpp"
#include "engine/math/Rect.hpp"
#include "engine/utils/Logger.hpp"
#include "glad/gl.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <vector>

namespace engine::graphics {

Renderer::Renderer() = default;

Renderer::~Renderer() = default;

void Renderer::Init(int width, int height) {
  if (width <= 0 || height <= 0) {
    Logger::Error("[Renderer::Init] invalid size %dx%d", width, height);
    return;
  }

  m_width = width;
  m_height = height;

  m_batch = std::make_unique<SpriteBatch>();

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

  EnsureRenderTargets();
  SetTileCounts(width, height);
}

void Renderer::Resize(int width, int height) {
  if (width == m_width && height == m_height)
    return;
  m_width = width;
  m_height = height;
  EnsureRenderTargets();
  SetTileCounts(width, height);
}

float Renderer::AspectRatio() const {
  return m_height > 0
             ? static_cast<float>(m_width) / static_cast<float>(m_height)
             : 1.0f;
}

glm::mat4 Renderer::UiProjection() const {
  return glm::ortho(0.0f, static_cast<float>(m_width),
                    static_cast<float>(m_height), 0.0f, -1.0f, 1.0f);
}

void Renderer::BeginFrame(const glm::mat4 &viewProjection,
                          const math::Rect &visibleBounds) {
  if (m_frameActive) {
    Logger::Warn("[Renderer] BeginFrame while frame active");
    return;
  }

  EnsureRenderTargets();

  m_viewProjection = viewProjection;
  m_visibleBounds = visibleBounds;

  m_drawList.Clear();
  m_pipeline.Reset();
  m_hasSprites = false;
  m_hasLights = false;
  m_blitOverride = AttachNone;
  m_frameActive = true;
}

void Renderer::EndFrame() {
  if (!m_frameActive) {
    Logger::Warn("[Renderer] EndFrame without Begin");
    return;
  }

  const bool wantComposite = m_hasSprites && m_hasLights;
  const bool wantSpriteOnly = m_hasSprites && !m_hasLights;
  const bool wantLightOnly = !m_hasSprites && m_hasLights;

  if (wantLightOnly) {
    if (m_pipeline.BeginStage(RenderStage::ClearDepth)) {
      ExecuteClearDepth();
      m_pipeline.EndStage(RenderStage::ClearDepth);
    }
  }

  if (m_hasSprites) {
    if (m_pipeline.BeginStage(RenderStage::Sprite)) {
      ExecuteSprite();
      m_pipeline.EndStage(RenderStage::Sprite);
    }
  }

  if (m_hasLights) {
    if (m_pipeline.BeginStage(RenderStage::Light)) {
      ExecuteLight();
      m_pipeline.EndStage(RenderStage::Light);
    }
  }

  if (m_blitOverride != AttachNone) {
    if (m_pipeline.BeginStage(RenderStage::Blit)) {
      ExecuteBlit(m_blitOverride);
      m_pipeline.EndStage(RenderStage::Blit);
    }
  } else if (wantComposite) {
    if (m_pipeline.BeginStage(RenderStage::Composite)) {
      ExecuteComposite();
      m_pipeline.EndStage(RenderStage::Composite);
    }
  } else if (wantSpriteOnly) {
    if (m_pipeline.BeginStage(RenderStage::Blit)) {
      ExecuteBlit(AttachSceneColor);
      m_pipeline.EndStage(RenderStage::Blit);
    }
  } else if (wantLightOnly) {
    if (m_pipeline.BeginStage(RenderStage::Blit)) {
      ExecuteBlit(AttachLightColor);
      m_pipeline.EndStage(RenderStage::Blit);
    }
  } else {
    Logger::Warn("[Renderer] EndFrame with no submission");
  }

  m_drawList.Clear();
  m_lightBuffer.pointCount = 0;
  m_lightBuffer.directionalCount = 0;
  m_lightBuffer.spotCount = 0;
  m_hasSprites = false;
  m_hasLights = false;
  m_blitOverride = AttachNone;
  m_pipeline.Reset();
  m_frameActive = false;
}

void Renderer::Draw(const glm::vec2 &position, const glm::vec2 &scale,
                    float rotation, const math::Color color,
                    const Texture *texture, const glm::vec2 &uvMin,
                    const glm::vec2 &uvMax, [[maybe_unused]] int layer) {
  m_drawList.Push(position, scale, rotation, color, texture, uvMin, uvMax);
  m_hasSprites = true;
}

void Renderer::RenderSprites(scene::World &world) {
  auto &reg = world.Registry();

  struct Cmd {
    const components::Transform *t;
    const components::Sprite *s;
    float yKey;
  };
  std::vector<Cmd> cmds;

  for (auto [e, s, t] : reg.View<components::Sprite, components::Transform>()) {
    const float hx = t.scale.x * 0.5f;
    const float hy = t.scale.y * 0.5f;
    float ex = hx;
    float ey = hy;

    if (t.rotation != 0.0f) {
      const float c = std::cos(t.rotation);
      const float sn = std::sin(t.rotation);
      ex = std::abs(hx * c) + std::abs(hy * sn);
      ey = std::abs(hx * sn) + std::abs(hy * c);
    }

    math::Rect eRect(t.position.x - ex, t.position.y - ey, ex * 2.0f,
                     ey * 2.0f);

    if (!m_visibleBounds.Intersects(eRect))
      continue;

    const float key = (s.ySortKey != 0.0f) ? s.ySortKey : t.position.y;
    cmds.push_back({&t, &s, key});
  }

  std::sort(cmds.begin(), cmds.end(), [](const Cmd &a, const Cmd &b) {
    if (a.s->layer != b.s->layer)
      return static_cast<int>(a.s->layer) < static_cast<int>(b.s->layer);
    return a.yKey < b.yKey;
  });

  for (const auto &c : cmds) {
    Draw(c.t->position, c.t->scale, c.t->rotation, c.s->color, c.s->texture,
         c.s->uvMin, c.s->uvMax, static_cast<int>(c.s->layer));
  }
}

void Renderer::RenderLights(scene::World &world) {
  auto &reg = world.Registry();

  m_lightBuffer.pointCount = 0;
  m_lightBuffer.directionalCount = 0;
  m_lightBuffer.spotCount = 0;

  for (auto [e, t, l] :
       reg.View<components::Transform, components::PointLight>()) {
    if (m_lightBuffer.pointCount >= LightBuffer::kMaxPoint)
      break;

    auto &g = m_lightBuffer.points[m_lightBuffer.pointCount++];
    g.position = glm::vec4(t.position.x, t.position.y, 0.0f, 1.0f);
    g.ambient = glm::vec4(l.ambient, 0.0f);
    g.diffuse = glm::vec4(l.diffuse, 0.0f);
    g.specular = glm::vec4(l.specular, 0.0f);
    g.attenuation = glm::vec4(l.constant, l.linear, l.quadratic, 0.0f);
  }

  for (auto [e, l] : reg.View<components::DirectionalLight>()) {
    if (m_lightBuffer.directionalCount >= LightBuffer::kMaxDirectional)
      break;

    auto &g = m_lightBuffer.directional[m_lightBuffer.directionalCount++];
    g.direction =
        glm::vec4(math::safeNormalize(l.direction, {0.0f, -1.0f, 0.0f}), 0.0f);
    g.color =
        math::Color::fromVec4(l.color.toVec4() * l.intensity).withAlpha(1.0f);
  }

  for (auto [e, t, l] :
       reg.View<components::Transform, components::SpotLight>()) {
    if (m_lightBuffer.spotCount >= LightBuffer::kMaxSpot)
      break;

    auto &g = m_lightBuffer.spots[m_lightBuffer.spotCount++];
    g.position = glm::vec4(t.position.x, t.position.y, 0.0f, 0.0f);
    g.direction =
        glm::vec4(math::safeNormalize(l.direction, {0.0f, -1.0f, 0.0f}), 0.0f);
    g.color =
        math::Color::fromVec4(l.color.toVec4() * l.intensity).withAlpha(1.0f);
    g.cone = glm::vec4(l.viewAngle, l.fadeAngle, 0.0f, 0.0f);

    const float r = (l.radius > 0.001f) ? l.radius : 15.0f;
    g.attenuation = glm::vec4(1.0f, 4.5f / r, 75.0f / (r * r), 0.0f);
  }

  m_hasLights =
      (m_lightBuffer.pointCount > 0 || m_lightBuffer.directionalCount > 0 ||
       m_lightBuffer.spotCount > 0);
}

void Renderer::ExecuteSprite() {
  m_sceneRT.Bind(0, 0, m_width, m_height);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  m_drawList.BuildBatches();

  const auto &groups = m_drawList.Groups();
  const auto &cmds = m_drawList.Commands();

  m_instanceScratch.resize(cmds.size());
  for (std::size_t i = 0; i < cmds.size(); ++i) {
    m_instanceScratch[i] = cmds[i].instance;
  }

  m_lastDrawCalls = 0;
  m_lastQuadCount = static_cast<std::uint32_t>(cmds.size());

  m_batch->Begin(m_viewProjection);

  for (const auto &g : groups) {
    m_batch->DrawInstances(m_instanceScratch.data() + g.start, g.count,
                           g.texture);
    ++m_lastDrawCalls;
  }

  m_batch->End();
}

void Renderer::ExecuteLight() {
  m_lightRT.Bind(0, 0, m_width, m_height);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  UpdateTileGrid(m_viewProjection);
  BuildTileGrid();

  m_lightRenderer.Upload(m_lightBuffer);

  m_lightShader.Bind();
  m_lightShader.SetMat4("uInViewProjection", glm::inverse(m_viewProjection));
  m_lightShader.SetVec3("uAmbientColor", m_ambientColor);
  m_lightShader.SetInt("uPointLightCount", m_lightBuffer.pointCount);
  m_lightShader.SetInt("uDirectionalLightCount",
                       m_lightBuffer.directionalCount);
  m_lightShader.SetInt("uSpotLightCount", m_lightBuffer.spotCount);
  m_lightShader.SetUInt("uTileCountX", m_tileCuller.TilesX());
  m_lightShader.SetUInt("uTileCountY", m_tileCuller.TilesY());
  m_lightShader.SetVec2("uTileWorldSize",
                        {m_tileCuller.TileWorldW(), m_tileCuller.TileWorldH()});
  m_lightShader.SetVec2("uWorldOrigin", m_tileCuller.WorldOrigin());

  // m_sceneRT.BindColorTexture(0);
  // m_lightShader.SetInt("uScene", 0);

  m_fullscreenQuad.Draw();
}

void Renderer::ExecuteComposite() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, m_width, m_height);
  glClear(GL_COLOR_BUFFER_BIT);

  m_compositeShader.Bind();
  m_sceneRT.BindColorTexture(0);
  m_lightRT.BindColorTexture(1);
  m_compositeShader.SetInt("uScene", 0);
  m_compositeShader.SetInt("uLight", 1);
  m_compositeShader.SetInt("uMode", 0);

  m_fullscreenQuad.Draw();
}

void Renderer::ExecuteBlit(Attachment src) {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, m_width, m_height);
  glClear(GL_COLOR_BUFFER_BIT);

  m_compositeShader.Bind();
  if (src == AttachLightColor) {
    m_lightRT.BindColorTexture(0);
    m_sceneRT.BindColorTexture(1);
    m_compositeShader.SetInt("uScene", 0); // binds light as "scene" source
    m_compositeShader.SetInt("uLight", 1);
    m_compositeShader.SetInt("uMode", 2); // mode=1 outputs uScene verbatim
  } else {
    m_sceneRT.BindColorTexture(0);
    m_lightRT.BindColorTexture(1);
    m_compositeShader.SetInt("uScene", 0);
    m_compositeShader.SetInt("uLight", 1);
    m_compositeShader.SetInt("uMode", 1);
  }

  m_fullscreenQuad.Draw();
}

void Renderer::ExecuteClearDepth() {
  m_sceneRT.Bind(0, 0, m_width, m_height);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::EnsureRenderTargets() {
  if (m_sceneRT.Width() != m_width || m_sceneRT.Height() != m_height) {
    m_sceneRT.Create(m_width, m_height);
    m_lightRT.Create(m_width, m_height);
  }
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

  m_tileCuller.SetTileGrid(tilesX, tilesY, worldW / static_cast<float>(tilesX),
                           worldH / static_cast<float>(tilesY), lo);
}

void Renderer::BuildTileGrid() {
  m_tileCuller.Build(m_lightBuffer.points, m_lightBuffer.pointCount);
  m_lightRenderer.UploadTileGrid(
      m_tileCuller.Offsets().data(), m_tileCuller.Offsets().size(),
      m_tileCuller.Indices().data(), m_tileCuller.Indices().size());
}

void Renderer::SetTileCounts(int width, int height) {
  constexpr std::uint32_t kTilePx = LightTileCuller::kTileSizePx;
  const std::uint32_t tx =
      std::max(1u, static_cast<std::uint32_t>(width) / kTilePx);
  const std::uint32_t ty =
      std::max(1u, static_cast<std::uint32_t>(height) / kTilePx);
  m_tileCuller.SetTileGrid(tx, ty, 1.0f, 1.0f,
                           {0.0f, 0.0f}); // size overwritten below
}

} // namespace engine::graphics

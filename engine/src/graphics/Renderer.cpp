#include "engine/graphics/Renderer.hpp"

#include "engine/graphics/SpriteBatch.hpp"
#include "engine/scene/Scene.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>

namespace engine {

namespace {
struct Quad {
  const Transform *transform = nullptr;
  const Sprite *sprite = nullptr;
  float yKey = 0.0f;
};

constexpr int kLayerCount = static_cast<int>(RenderLayer::COUNT);

// Exact AAB half-extents of a rotated quad.
inline void HalfExtents(const Transform &t, float &ex, float &ey) {
  const float hx = t.scale.x * 0.5f;
  const float hy = t.scale.y * 0.5f;
  if (t.rotation == 0.0f) {
    ex = hx;
    ey = hy;
    return;
  }
  const float c = std::cos(t.rotation);
  const float s = std::sin(t.rotation);
  ex = std::abs(hx * c) + std::abs(hy * s);
  ey = std::abs(hx * s) + std::abs(hy * c);
}
} // namespace

Renderer::Renderer(std::size_t maxQuads)
    : m_batch(std::make_unique<SpriteBatch>(maxQuads)) {}

Renderer::~Renderer() = default;

void Renderer::Begin(const Camera &camera) {
  m_camera = &camera;
  m_lastCulled = 0;
  m_batch->Begin(camera);
}

void Renderer::End() {
  m_batch->End();
  m_camera = nullptr;
}

void Renderer::Draw(const Transform &t, const Sprite &s) {
  m_batch->DrawQuad(t, s);
}

void Renderer::DrawScene(const Scene &scene) {
  if (!m_camera)
    return;

  // render layersh
  std::array<std::vector<Quad>, 6> layers;
  std::uint32_t culled = 0;

  WorldBounds view{};
  if (m_cullEnabled)
    view = m_camera->VisibleWorldBounds();

  scene.ForEach([&](Entity, const Transform &t, const Sprite &s) {
    if (m_cullEnabled) {
      float ex, ey;
      HalfExtents(t, ex, ey);
      if (!view.Overlaps(t.position.x - ex, t.position.x + ex,
                         t.position.y - ey, t.position.y + ey)) {
        ++culled;
        return;
      }
    }

    const int li = static_cast<int>(s.layer);
    if (li < 0 || li >= kLayerCount)
      return;

    const float key = (s.ySortKey != 0.0f) ? s.ySortKey : t.position.y;
    layers[li].push_back(Quad{&t, &s, key});
  });

  for (auto &bucket : layers) {
    std::sort(bucket.begin(), bucket.end(),
              [](const Quad &a, const Quad &b) { return a.yKey < b.yKey; });
    for (const auto &q : bucket) {
      m_batch->DrawQuad(*q.transform, *q.sprite);
    }
  }

  m_lastCulled = culled;
}

std::uint32_t Renderer::DrawCalls() const { return m_batch->LastDrawCalls(); }
std::uint32_t Renderer::QuadCount() const { return m_batch->LastQuadCount(); }
std::uint32_t Renderer::CulledCount() const { return m_lastCulled; }

} // namespace engine

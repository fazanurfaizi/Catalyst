#include "engine/graphics/Renderer.hpp"

#include "engine/graphics/SpriteBatch.hpp"
#include "engine/scene/Scene.hpp"
#include <algorithm>
#include <array>

namespace engine {

namespace {
struct Quad {
  const Transform *transform = nullptr;
  const Sprite *sprite = nullptr;
  float yKey = 0.0f;
};

constexpr int kLayerCount = static_cast<int>(RenderLayer::COUNT);
} // namespace

Renderer::Renderer(std::size_t maxQuads)
    : m_batch(std::make_unique<SpriteBatch>(maxQuads)) {}

Renderer::~Renderer() = default;

void Renderer::Begin(const Camera &camera) { m_batch->Begin(camera); }

void Renderer::End() { m_batch->End(); }

void Renderer::Draw(const Transform &t, const Sprite &s) {
  m_batch->DrawQuad(t, s);
}

void Renderer::DrawScene(const Scene &scene) {
  // render layersh
  std::array<std::vector<Quad>, 6> layers;

  scene.ForEach([&](Entity, const Transform &t, const Sprite &s) {
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
}

std::uint32_t Renderer::DrawCalls() const { return m_batch->LastDrawCalls(); }
std::uint32_t Renderer::QuadCount() const { return m_batch->LastQuadCount(); }

} // namespace engine

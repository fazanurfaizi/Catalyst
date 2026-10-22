#include "engine/graphics/Renderer.hpp"

#include "engine/graphics/SpriteBatch.hpp"
#include "engine/scene/Scene.hpp"

namespace engine {

Renderer::Renderer(std::size_t maxQuads)
    : m_batch(std::make_unique<SpriteBatch>(maxQuads)) {}

Renderer::~Renderer() = default;

void Renderer::Begin(const Camera &camera) { m_batch->Begin(camera); }

void Renderer::End() { m_batch->End(); }

void Renderer::Draw(const Transform &t, const Sprite &s) {
  m_batch->DrawQuad(t, s);
}

void Renderer::DrawScene(const Scene &scene) {
  scene.ForEach([this](Entity, const Transform &t, const Sprite &s) {
    m_batch->DrawQuad(t, s);
  });
}

std::uint32_t Renderer::DrawCalls() const { return m_batch->LastDrawCalls(); }
std::uint32_t Renderer::QuadCount() const { return m_batch->LastQuadCount(); }

} // namespace engine

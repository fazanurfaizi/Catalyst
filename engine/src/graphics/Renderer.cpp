#include "engine/graphics/Renderer.hpp"

#include "engine/graphics/SpriteBatch.hpp"
#include <cmath>
#include <cstdlib>

namespace engine::graphics {

Renderer::Renderer(std::size_t maxQuads)
    : m_batch(std::make_unique<SpriteBatch>(maxQuads)) {}

Renderer::~Renderer() = default;

void Renderer::Begin(const glm::mat4 &viewProjection) {
  m_viewProjection = viewProjection;
  m_batch->Begin(m_viewProjection);
}

void Renderer::End() { m_batch->End(); }

void Renderer::Draw(const glm::vec2 &position, const glm::vec2 &scale,
                    float rotation, const math::Color color,
                    const Texture *texture, const glm::vec2 &uvMin,
                    const glm::vec2 &uvMax, int layer) {
  m_batch->DrawQuad(position, scale, rotation, color, texture, uvMin, uvMax);
}

std::uint32_t Renderer::DrawCalls() const { return m_batch->LastDrawCalls(); }
std::uint32_t Renderer::QuadCount() const { return m_batch->LastQuadCount(); }

} // namespace engine::graphics

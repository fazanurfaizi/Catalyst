#pragma once

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
  explicit Renderer(std::size_t maxQuads = 4096);
  ~Renderer();

  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

  void Begin(const glm::mat4 &viewProjection);
  void End();

  void Draw(const glm::vec2 &position, const glm::vec2 &scale, float rotation,
            const math::Color color, const Texture *texture,
            const glm::vec2 &uvMin, const glm::vec2 &uvMax, int layer);

  std::uint32_t DrawCalls() const;
  std::uint32_t QuadCount() const;

private:
  std::unique_ptr<SpriteBatch> m_batch;
  glm::mat4 m_viewProjection{1.0f};
};

} // namespace engine::graphics

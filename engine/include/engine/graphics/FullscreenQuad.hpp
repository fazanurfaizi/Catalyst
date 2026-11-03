#pragma once

#include <cstdint>

namespace engine::graphics {

/**
 * A single NDC-space triangle-pair with attribute location 0 = vec2 position.
 * Fragment shaders compute UV = aPos * 0.5 + 0.5.
 */
class FullscreenQuad {
public:
  FullscreenQuad();
  ~FullscreenQuad();
  FullscreenQuad(const FullscreenQuad &) = delete;
  FullscreenQuad &operator=(const FullscreenQuad &) = delete;

  void Draw() const;

private:
  std::uint32_t m_vao = 0;
  std::uint32_t m_vbo = 0;
  std::uint32_t m_ebo = 0;
};

} // namespace engine::graphics

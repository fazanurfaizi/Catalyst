#pragma once

#include "engine/graphics/Shader.hpp"
#include "engine/graphics/Texture.hpp"
#include "glad/gl.h"
#include <cstddef>
#include <cstdint>
#include <glm/glm.hpp>

namespace engine::graphics {

struct SpriteInstance;
struct SpriteCommand;

class SpriteBatch {
public:
  SpriteBatch();
  ~SpriteBatch();

  // SpriteBatch(const SpriteBatch &) = delete;
  // SpriteBatch &operator=(const SpriteBatch &) = delete;
  //
  // SpriteBatch(SpriteBatch &&) noexcept;
  // SpriteBatch &operator=(SpriteBatch &&) noexcept;

  void Release();

  void Begin(const glm::mat4 &viewProjection);

  void DrawInstances(const SpriteInstance *data, std::size_t count,
                     const Texture *texture);

  // void DrawQuad(const glm::vec2 &position, const glm::vec2 &scale,
  //               float rotation, const math::Color &color,
  //               const Texture *texture, const glm::vec2 &uvMin,
  //               const glm::vec2 &uvMax);

  void End();

  std::uint32_t LastDrawCalls() const { return m_lastDrawCalls; }
  std::uint32_t LastInstanceCount() const { return m_lastInstanceCount; }

  Shader &GetShader() { return m_shader; }

private:
  GLuint m_vao = 0;
  GLuint m_staticVbo = 0;
  GLuint m_ibo = 0;
  GLuint m_instanceVbo = 0;

  Shader m_shader;

  glm::mat4 m_viewProjection{1.0f};

  Texture m_defaultTexture;

  std::uint32_t m_lastDrawCalls = 0;
  std::uint32_t m_lastInstanceCount = 0;
};

} // namespace engine::graphics

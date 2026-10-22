#pragma once

#include "engine/graphics/Camera.hpp"
#include "engine/graphics/Shader.hpp"
#include "engine/graphics/Texture.hpp"
#include "engine/scene/Scene.hpp"
#include "glad/gl.h"
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

namespace engine {

class SpriteBatch {
public:
  explicit SpriteBatch(std::size_t maxQuads);
  SpriteBatch(SpriteBatch &&) noexcept;
  SpriteBatch(const SpriteBatch &) = delete;
  SpriteBatch &operator=(SpriteBatch &&) noexcept;
  SpriteBatch &operator=(const SpriteBatch &) = delete;
  ~SpriteBatch();

  void Begin(const Camera &camera);

  void DrawQuad(const Transform &t, const Sprite &s);

  void End();

  void Render(const Scene &scene, const Camera &camera);

  std::uint32_t LastDrawCalls() const { return m_LastDrawCalls; }
  std::uint32_t LastQuadCount() const { return m_LastQuadCount; }

private:
  struct Vertex {
    glm::vec2 position;
    glm::vec2 uv;
    glm::vec4 color;
  };

  void Flush();

  GLuint m_Vao = 0;
  GLuint m_Vbo = 0;
  GLuint m_Ibo = 0;
  std::uint32_t m_MaxQuads = 0;

  std::vector<Vertex> m_Vertices;
  std::uint32_t m_QuadCount = 0;

  Shader m_Shader;
  const Camera *m_Camera = nullptr;

  Texture m_DefaultTexture;
  const Texture *m_CurrentTexture = nullptr;

  std::uint32_t m_LastDrawCalls = 0;
  std::uint32_t m_LastQuadCount = 0;
};

} // namespace engine

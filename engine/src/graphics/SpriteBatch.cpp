#include "engine/graphics/SpriteBatch.hpp"
#include "engine/core/Logger.hpp"
#include "glad/gl.h"

#include <cmath>
#include <cstddef>
#include <vector>

namespace engine {

namespace {
constexpr const char *kVertexShaderSrc = R"glsl(
#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

uniform mat4 uViewProjection;

out vec2 vUV;
out vec4 vColor;

void main() {
  gl_Position = uViewProjection * vec4(aPos, 0.0, 1.0);
  vUV = aUV;
  vColor = aColor;
}
)glsl";

constexpr const char *kFragmentShaderSrc = R"glsl(
#version 330 core

in vec2 vUV;
in vec4 vColor;

uniform sampler2D uTexture;

out vec4 FragColor;

void main() {
  vec4 tex = texture(uTexture, vUV);
  FragColor = tex * vColor;
}
)glsl";

} // namespace

SpriteBatch::SpriteBatch(std::size_t maxQuads)
    : m_MaxQuads(static_cast<std::uint32_t>(maxQuads)) {
  if (m_MaxQuads == 0)
    m_MaxQuads = 1;

  if (!m_Shader.Compile(kVertexShaderSrc, kFragmentShaderSrc)) {
    Logger::Error("[SpriteBatch] shader compilation failed");
    return;
  }

  m_Vertices.resize(m_MaxQuads * 4);

  // Index buffer -> two triangles per quad
  std::vector<std::uint32_t> indices(m_MaxQuads * 6);
  for (std::size_t i = 0; i < m_MaxQuads; ++i) {
    const std::uint32_t v = i * 4;
    indices[i * 6 + 0] = v + 0;
    indices[i * 6 + 1] = v + 1;
    indices[i * 6 + 2] = v + 2;
    indices[i * 6 + 3] = v + 2;
    indices[i * 6 + 4] = v + 3;
    indices[i * 6 + 5] = v + 0;
  }

  glGenVertexArrays(1, &m_Vao);
  glBindVertexArray(m_Vao);

  glGenBuffers(1, &m_Vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(m_Vertices.size() * sizeof(Vertex)),
               nullptr, GL_DYNAMIC_DRAW);

  glGenBuffers(1, &m_Ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(indices.size() * sizeof(std::uint32_t)),
               indices.data(), GL_STATIC_DRAW);

  constexpr GLsizei stride = sizeof(Vertex);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
      0, 2, GL_FLOAT, GL_FALSE, stride,
      reinterpret_cast<const void *>(offsetof(Vertex, position)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                        reinterpret_cast<const void *>(offsetof(Vertex, uv)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(
      2, 4, GL_FLOAT, GL_FALSE, stride,
      reinterpret_cast<const void *>(offsetof(Vertex, color)));

  glBindVertexArray(0);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  m_DefaultTexture.CreateSolid(0xFFFFFFFF);
}

SpriteBatch::~SpriteBatch() {
  if (m_Ibo)
    glDeleteBuffers(1, &m_Ibo);
  if (m_Vbo)
    glDeleteBuffers(1, &m_Vbo);
  if (m_Vao)
    glDeleteVertexArrays(1, &m_Vao);
}

SpriteBatch::SpriteBatch(SpriteBatch &&other) noexcept
    : m_Vao(other.m_Vao), m_Vbo(other.m_Vbo), m_Ibo(other.m_Ibo),
      m_MaxQuads(other.m_MaxQuads), m_Vertices(std::move(other.m_Vertices)),
      m_QuadCount(other.m_QuadCount), m_Shader(std::move(other.m_Shader)),
      m_viewProjection(other.m_viewProjection) {
  other.m_Vao = other.m_Vbo = other.m_Ibo = 0;
  other.m_MaxQuads = 0;
  other.m_QuadCount = 0;
}

SpriteBatch &SpriteBatch::operator=(SpriteBatch &&other) noexcept {
  if (this != &other) {
    if (m_Ibo)
      glDeleteBuffers(1, &m_Ibo);
    if (m_Vbo)
      glDeleteBuffers(1, &m_Vbo);
    if (m_Vao)
      glDeleteVertexArrays(1, &m_Vao);

    m_Vao = other.m_Vao;
    m_Vbo = other.m_Vbo;
    m_Ibo = other.m_Ibo;
    m_MaxQuads = other.m_MaxQuads;
    m_Vertices = std::move(other.m_Vertices);
    m_QuadCount = other.m_QuadCount;
    m_Shader = std::move(other.m_Shader);
    m_viewProjection = other.m_viewProjection;

    other.m_Vao = other.m_Vbo = other.m_Ibo = 0;
    other.m_MaxQuads = 0;
    other.m_QuadCount = 0;
  }
  return *this;
}

void SpriteBatch::Begin(const glm::mat4 &viewProjection) {
  m_viewProjection = viewProjection;
  m_QuadCount = 0;
  m_CurrentTexture = nullptr;
  m_LastDrawCalls = 0;
  m_LastQuadCount = 0;
}

void SpriteBatch::DrawQuad(const glm::vec2 &position, const glm::vec2 &scale,
                           float rotation, const Color &color,
                           const Texture *texture, const glm::vec2 &uvMin,
                           const glm::vec2 &uvMax) {
  const Texture *tex = texture ? texture : &m_DefaultTexture;

  if (tex != m_CurrentTexture) {
    Flush();
    m_QuadCount = 0;
    m_CurrentTexture = tex;
  }

  if (m_QuadCount >= m_MaxQuads) {
    Flush();
    m_QuadCount = 0;
  }

  const float c = std::cos(rotation);
  const float sn = std::sin(rotation);
  const glm::vec2 half = scale * 0.5f;

  auto rotate = [&](glm::vec2 p) {
    return glm::vec2(p.x * c - p.y * sn, p.x * sn + p.y * c);
  };

  const glm::vec2 corners[4] = {
      rotate({-half.x, -half.y}) + position,
      rotate({half.x, -half.y}) + position,
      rotate({half.x, half.y}) + position,
      rotate({-half.x, half.y}) + position,
  };
  const glm::vec2 uvs[4] = {
      {uvMin.x, uvMin.y}, // BL  (was {0,1})
      {uvMax.x, uvMin.y}, // BR  (was {1,1})
      {uvMax.x, uvMax.y}, // TR  (was {1,0})
      {uvMin.x, uvMax.y}, // TL  (was {0,0})
  };

  Vertex *dst = &m_Vertices[m_QuadCount * 4];
  for (int i = 0; i < 4; ++i) {
    dst[i].position = corners[i];
    dst[i].uv = uvs[i];
    dst[i].color = color.toVec4();
  }
  ++m_QuadCount;
}

void SpriteBatch::Flush() {
  if (m_QuadCount == 0)
    return;

  glBindVertexArray(m_Vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0,
                  static_cast<GLsizeiptr>(m_QuadCount * 4 * sizeof(Vertex)),
                  m_Vertices.data());

  m_Shader.Bind();
  m_Shader.SetMat4("uViewProjection", m_viewProjection);

  const Texture *tex = m_CurrentTexture ? m_CurrentTexture : &m_DefaultTexture;
  tex->Bind(0);
  m_Shader.SetInt("uTexture", 0);

  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_QuadCount * 6),
                 GL_UNSIGNED_INT, nullptr);

  glBindVertexArray(0);
  ++m_LastDrawCalls;
  m_LastQuadCount += m_QuadCount;
}

void SpriteBatch::End() {
  Flush();
  m_QuadCount = 0;
  m_CurrentTexture = nullptr;
}

} // namespace engine

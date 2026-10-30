#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <map>
#include <string>

namespace engine::graphics {

struct Glyph {
  GLuint textureId;
  glm::ivec2 size;
  glm::ivec2 bearing;
  float advance;
  glm::vec2 uvMin;
  glm::vec2 uvMax;
};

class Font {
public:
  Font() = default;
  ~Font() = default;

  bool LoadFromFile(const std::string &path, float fontSize);

  const Glyph *FindGlyph(std::uint32_t codepoint) const;

  const Glyph &GetCharacter(char c) const;
  GLuint GetTextureId() const { return m_textureId; }

  float GetAscent() const { return m_ascent; }
  float GetDescent() const { return m_descent; }
  float GetLineHeight() const { return m_ascent - m_descent + m_lineGap; }

private:
  GLuint m_textureId = 0;
  std::map<char, Glyph> m_glyphs;
  float m_ascent = 0.0f;
  float m_descent = 0.0f;
  float m_lineGap = 0.0f;
};

} // namespace engine::graphics

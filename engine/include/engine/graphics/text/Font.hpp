#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <map>

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
  ~Font();

  Font(const Font &) = delete;
  Font &operator=(const Font &) = delete;
  Font(Font &&) noexcept;
  Font &operator=(Font &&) noexcept;

  bool UploadAtlasFromR8(std::uint32_t width, std::uint32_t height,
                         const std::uint8_t *coverage);

  void SetGlyph(std::uint32_t codepoint, Glyph glyph);

  const Glyph *FindGlyph(std::uint32_t codepoint) const;

  const Glyph &GetCharacter(char c) const;

  GLuint GetTextureId() const noexcept { return m_textureId; }
  float GetAscent() const noexcept { return m_ascent; }
  float GetDescent() const noexcept { return m_descent; }
  float GetLineHeight() const noexcept {
    return m_ascent - m_descent + m_lineGap;
  }

  std::map<std::uint32_t, Glyph> &Glyphs() { return m_glyphs; }

  void SetMetrics(float ascent, float descent, float lineGap) {
    m_ascent = ascent;
    m_descent = descent;
    m_lineGap = lineGap;
  }

private:
  void Release();

  GLuint m_textureId = 0;
  std::map<std::uint32_t, Glyph> m_glyphs;
  float m_ascent = 0.0f;
  float m_descent = 0.0f;
  float m_lineGap = 0.0f;
};

} // namespace engine::graphics

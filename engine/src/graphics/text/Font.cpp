#include "engine/graphics/text/Font.hpp"
#include "engine/utils/Logger.hpp"
#include "glad/gl.h"

namespace engine::graphics {

Font::~Font() { Release(); }

Font::Font(Font &&other) noexcept
    : m_textureId(other.m_textureId), m_glyphs(std::move(other.m_glyphs)),
      m_ascent(other.m_ascent), m_descent(other.m_descent),
      m_lineGap(other.m_lineGap) {
  other.m_textureId = 0;
  other.m_ascent = other.m_descent = other.m_lineGap = 0.0f;
}

Font &Font::operator=(Font &&other) noexcept {
  if (this != &other) {
    Release();
    m_textureId = other.m_textureId;
    m_glyphs = std::move(other.m_glyphs);
    m_ascent = other.m_ascent;
    m_descent = other.m_descent;
    m_lineGap = other.m_lineGap;

    other.m_textureId = 0;
    other.m_ascent = other.m_descent = other.m_lineGap = 0.0f;
  }
  return *this;
}

void Font::Release() {
  if (m_textureId) {
    glDeleteTextures(1, &m_textureId);
    m_textureId = 0;
  }
  m_glyphs.clear();
}

bool Font::UploadAtlasFromR8(std::uint32_t width, std::uint32_t height,
                             const std::uint8_t *coverage) {
  if (width == 0 || height == 0 || coverage == nullptr) {
    Logger::Error("[Font] UploadAtlasFromR8: empty atlas");
    return false;
  }

  if (m_textureId) {
    glDeleteTextures(1, &m_textureId);
    m_textureId = 0;
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glGenTextures(1, &m_textureId);
  glBindTexture(GL_TEXTURE_2D, m_textureId);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, static_cast<GLsizei>(width),
               static_cast<GLsizei>(height), 0, GL_RED, GL_UNSIGNED_BYTE,
               coverage);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_2D, 0);

  if (m_textureId == 0) {
    Logger::Error("[Font] UploadAtlasFromR8: glGenTextures failed");
    return false;
  }
  return true;
}

void Font::SetGlyph(std::uint32_t codepoint, Glyph glyph) {
  glyph.textureId = m_textureId;
  m_glyphs[codepoint] = std::move(glyph);
}

const Glyph *Font::FindGlyph(std::uint32_t codepoint) const {
  auto it = m_glyphs.find(codepoint);
  return it != m_glyphs.end() ? &it->second : nullptr;
}

const Glyph &Font::GetCharacter(char c) const {
  return m_glyphs.at(static_cast<std::uint32_t>(static_cast<unsigned char>(c)));
}

} // namespace engine::graphics

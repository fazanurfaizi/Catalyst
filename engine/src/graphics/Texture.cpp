#include "engine/graphics/Texture.hpp"

#include "engine/utils/Logger.hpp"

#include <glad/gl.h>
#include <stb/stb_image.h>

#include <vector>

namespace engine::graphics {

Texture::~Texture() { Release(); }

Texture::Texture(Texture &&other) noexcept
    : m_Id(other.m_Id), m_Width(other.m_Width), m_Height(other.m_Height) {
  other.m_Id = 0;
  other.m_Width = 0;
  other.m_Height = 0;
}

Texture &Texture::operator=(Texture &&other) noexcept {
  if (this != &other) {
    Release();
    m_Id = other.m_Id;
    m_Width = other.m_Width;
    m_Height = other.m_Height;
    other.m_Id = 0;
    other.m_Width = other.m_Height = 0;
  }
  return *this;
}

void Texture::Release() {
  if (m_Id) {
    glDeleteTextures(1, &m_Id);
    m_Id = 0;
  }
  m_Width = 0;
  m_Height = 0;
}

bool Texture::LoadFromFile(const std::string &path) {
  stbi_set_flip_vertically_on_load(true);

  int w = 0;
  int h = 0;
  int channels = 0;
  stbi_uc *data = stbi_load(path.c_str(), &w, &h, &channels, STBI_rgb_alpha);
  if (!data) {
    const char *dsc = stbi_failure_reason() ? stbi_failure_reason() : "?";
    Logger::Error("[Texture] failed to load: %s (%s)", path.c_str(), dsc);
    return false;
  }

  Upload(w, h, data, /*generateMipmaps=*/true);
  stbi_image_free(data);
  return true;
}

void Texture::CreateSolid(std::uint32_t rgba) {
  const std::uint8_t px[4] = {
      static_cast<std::uint8_t>((rgba >> 24) & 0xFF),
      static_cast<std::uint8_t>((rgba >> 16) & 0xFF),
      static_cast<std::uint8_t>((rgba >> 8) & 0xFF),
      static_cast<std::uint8_t>((rgba >> 0) & 0xFF),
  };
  Upload(1, 1, px, /*generateMipmaps=*/false);
}

void Texture::CreateCheckerboard(int cells, std::uint32_t colorA,
                                 std::uint32_t colorB) {
  if (cells < 1)
    cells = 1;

  constexpr int kCellPx = 8;
  const int size = cells * kCellPx;
  std::vector<std::uint8_t> pixels(static_cast<std::size_t>(size) * size * 4);

  auto unpack = [](std::uint32_t c, std::uint8_t out[4]) {
    out[0] = static_cast<std::uint8_t>((c >> 24) & 0xFF);
    out[1] = static_cast<std::uint8_t>((c >> 16) & 0xFF);
    out[2] = static_cast<std::uint8_t>((c >> 8) & 0xFF);
    out[3] = static_cast<std::uint8_t>((c >> 0) & 0xFF);
  };

  std::uint8_t a[4], b[4];
  unpack(colorA, a);
  unpack(colorB, b);

  for (int y = 0; y < size; ++y) {
    for (int x = 0; x < size; ++x) {
      const bool even = ((x / kCellPx) + (y / kCellPx)) % 2 == 0;
      const std::uint8_t *src = even ? a : b;
      const std::size_t idx = (static_cast<std::size_t>(y) * size + x) * 4;
      pixels[idx + 0] = src[0];
      pixels[idx + 1] = src[1];
      pixels[idx + 2] = src[2];
      pixels[idx + 3] = src[3];
    }
  }
  Upload(size, size, pixels.data(), /*generateMipmaps=*/true);
}

void Texture::Upload(int width, int height, const std::uint8_t *pixels,
                     bool generateMipmaps) {
  if (width <= 0 || height <= 0 || !pixels)
    return;

  if (m_Id) {
    glBindTexture(GL_TEXTURE_2D, m_Id);
  } else {
    glGenTextures(1, &m_Id);
    glBindTexture(GL_TEXTURE_2D, m_Id);
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, pixels);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  if (generateMipmaps) {
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  m_Width = width;
  m_Height = height;
}

void Texture::Bind(std::uint32_t unit) const {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, m_Id);
}

} // namespace engine::graphics

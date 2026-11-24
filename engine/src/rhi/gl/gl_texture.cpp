#include "engine/rhi/gl/gl_texture.hpp"
#include "engine/utils/Logger.hpp"

#include <cassert>
#include <cstdlib>

namespace engine::rhi {

namespace {

struct GlFormatTraits {
  GLenum internalFormat;
  GLenum format;
  GLenum type;
  std::uint32_t bytesPerPixel;
};

GlFormatTraits ToGlFormat(const Format format) {
  switch (format) {
  case Format::R8_Unorm:
    return {GL_R8, GL_RED, GL_UNSIGNED_BYTE, 1};
  case Format::RGB8_Unorm:
    return {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, 3};
  case Format::RGBA8_Unorm:
    return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 4};
  case Format::RGBA8_Srgb:
    return {GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE, 4};
  case Format::RGBA16_Float:
    return {GL_RGBA16F, GL_RGBA, GL_FLOAT, 8};
  case Format::R32_Float:
    return {GL_R32F, GL_RED, GL_FLOAT, 4};
  case Format::RGBA32_Float:
    return {GL_RGBA32F, GL_RGBA, GL_FLOAT, 16};
  case Format::D24_Unorm_S8_Uint:
    return {GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 4};
  case Format::Undefined:
    break;
  }

  Logger::Error("[RHI/GL] texture created with Format::Undefined");
  std::abort();
}

[[nodiscard]] std::uint32_t MipLevelCount(const TextureCreateInfo &createInfo) {
  if (!createInfo.mipmaps) {
    return 1;
  }

  const std::uint32_t maxDimension = std::max(createInfo.width, createInfo.height);
  std::uint32_t levels = 1;
  for (std::uint32_t size = maxDimension; size > 1; size >>= 1) {
    ++levels;
  }
  return levels;
}

[[nodiscard]] GLTexture createGlTexture(Device &device, const TextureCreateInfo &createInfo) {
  static_cast<void>(device);

  const GlFormatTraits glFormat = ToGlFormat(createInfo.format);
  const std::uint32_t levels = MipLevelCount(createInfo);

  GLuint name{0};
  glCreateTextures(GL_TEXTURE_2D, 1, &name);
  if (name == 0) {
    Logger::Error("[RHI/GL] glCreateTextures failed for texture '%s'",
                  createInfo.debugName != nullptr ? createInfo.debugName : "<unnamed>");
    std::abort();
  }

  if (createInfo.sampleCount > 1) {
    glTextureStorage2DMultisample(name, static_cast<GLsizei>(createInfo.sampleCount),
                                  glFormat.internalFormat, static_cast<GLsizei>(createInfo.width),
                                  static_cast<GLsizei>(createInfo.height), GL_TRUE);
  } else {
    glTextureStorage2D(name, static_cast<GLsizei>(levels), glFormat.internalFormat,
                       static_cast<GLsizei>(createInfo.width),
                       static_cast<GLsizei>(createInfo.height));
  }

  return GLTexture{name};
}

} // namespace

TextureImpl::TextureImpl(Device &device, const TextureCreateInfo &createInfo)
    : m_device(device), m_handle(createGlTexture(device, createInfo)), m_target(GL_TEXTURE_2D),
      m_format(createInfo.format), m_width(createInfo.width), m_height(createInfo.height),
      m_levels(MipLevelCount(createInfo)) {
  assert(m_width > 0 && m_height > 0);
  assert(m_levels >= 1);
}

void TextureImpl::SetData(const void *data, std::uint32_t size, std::uint32_t level) {
  assert(data != nullptr);
  assert(level < m_levels);
  assert(m_levels <= 1 || level < m_levels);

  const GlFormatTraits glFormat = ToGlFormat(m_format);
  const std::uint32_t levelWidth = std::max(1u, m_width >> level);
  const std::uint32_t levelHeight = std::max(1u, m_height >> level);
  const std::uint32_t expectedSize = levelWidth * levelHeight * glFormat.bytesPerPixel;

  if (size < expectedSize) {
    Logger::Error("[RHI/GL] texture upload is %u bytes, level %u needs %u (%ux%u)", size, level,
                  expectedSize, levelWidth, levelHeight);
    return;
  }

  glTextureSubImage2D(m_handle.GlName(), static_cast<GLint>(level), 0, 0,
                      static_cast<GLsizei>(levelWidth), static_cast<GLsizei>(levelHeight),
                      glFormat.format, glFormat.type, data);
}

void TextureImpl::GenerateMipmaps() {
  if (m_levels <= 1) {
    return;
  }
  glGenerateTextureMipmap(m_handle.GlName());
}

} // namespace engine::rhi

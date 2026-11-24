#pragma once

#include "engine/rhi/Texture.hpp"

#include "engine/rhi/gl/gl_objects.hpp"
#include "glad/gl.h"

#include <cstdint>

namespace engine::rhi {

class TextureImpl final {
public:
  TextureImpl(Device &device, const TextureCreateInfo &createInfo);
  ~TextureImpl() = default;

  TextureImpl(const TextureImpl &) = delete;
  TextureImpl &operator=(const TextureImpl &) = delete;

  void SetData(const void *data, std::uint32_t size, std::uint32_t level);
  void GenerateMipmaps();

  std::uint32_t Width() const { return m_width; }
  std::uint32_t Height() const { return m_height; }
  Format GetFormat() const { return m_format; }

  GLenum GlTarget() const { return m_target; }

private:
  Device &m_device;
  GLTexture m_handle;
  GLenum m_target{GL_TEXTURE_2D};
  Format m_format{Format::RGBA8_Unorm};
  std::uint32_t m_width{0}, m_height{0};
  std::uint32_t m_levels{1};
};

} // namespace engine::rhi

#pragma once

#include "engine/rhi/Enums.hpp"
#include <cstdint>
#include <memory>

namespace engine::rhi {

class Device;
class TextureImpl;

class TextureCreateInfo {
public:
  Format format{Format::RGBA8_Unorm};
  std::uint32_t width{1};
  std::uint32_t height{1};
  TextureUsage usage{TextureUsage::Sampled};
  bool mipmaps{false};
  std::uint32_t sampleCount{1};
  const char *debugName{nullptr};
};

class Texture {
public:
  Texture(Device &device, const TextureCreateInfo &createInfo);
  ~Texture();

  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;

  void SetData(const void *data, std::uint32_t size, std::uint32_t level = 0);
  void GenerateMipmaps();

  [[nodiscard]] std::uint32_t Width() const;
  [[nodiscard]] std::uint32_t Height() const;
  [[nodiscard]] Format GetFormat() const;
  [[nodiscard]] TextureImpl &GetImpl();

private:
  std::unique_ptr<TextureImpl> m_impl;
};

} // namespace engine::rhi

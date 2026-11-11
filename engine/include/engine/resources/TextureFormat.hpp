#pragma once

#include <cstdint>

namespace engine::resources {

enum class ImageFormat {
  None = 0,
  R8,
  RGB8,
  RGBA8,
  RGBA32F,
};

struct TextureSpecification {
  std::uint32_t width = 1;
  std::uint32_t height = 1;
  ImageFormat format = ImageFormat::RGBA8;
  bool magFilterLinear = false;
};

} // namespace engine::resources

#pragma once

#include "engine/rhi/Enums.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace engine::rhi {

class Texture;

class SwapChain {
public:
  virtual ~SwapChain() = default;
  virtual void Resize(std::uint32_t width, std::uint32_t height) = 0;
  virtual void Present() = 0;

  [[nodiscard]] virtual Texture *CurrentTexture() = 0;
  [[nodiscard]] virtual std::uint32_t Width() const = 0;
  [[nodiscard]] virtual std::uint32_t Height() const = 0;
  [[nodiscard]] virtual Format ColorFormat() const = 0;

  virtual void RequestCapture() = 0;
  virtual bool ReadBackCapture(std::uint32_t &outWidth,
                               std::uint32_t &outHeight,
                               std::vector<std::byte> &outRgba8) = 0;
};

} // namespace engine::rhi

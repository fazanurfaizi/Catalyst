#pragma once

#include "engine/rhi/SwapChain.hpp"

namespace engine::rhi {

class GLSwapChain final : public SwapChain {
public:
  void Resize(std::uint32_t width, std::uint32_t height) override;

  void Present() override;

  [[nodiscard]] Texture *CurrentTexture() override;

  [[nodiscard]] std::uint32_t Width() const override;

  [[nodiscard]] std::uint32_t Height() const override;

  [[nodiscard]] Format ColorFormat() const override;

  void RequestCapture() override;

  bool ReadBackCapture(std::uint32_t &outWidth, std::uint32_t &outHeight, std::vector<std::byte> &outRgba8) override;
};

} // namespace engine::rhi

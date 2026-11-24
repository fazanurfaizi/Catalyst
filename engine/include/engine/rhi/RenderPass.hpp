#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace engine::rhi {

class Device;
class RenderPassImpl;
class Texture;

enum class LoadAction : std::uint8_t { DontCare, Clear, Load };
enum class StoreAction : std::uint8_t { DontCare, Store };

class ColorAttachment {
public:
  const Texture *texture{nullptr};
  LoadAction load{LoadAction::Clear};
  StoreAction store{StoreAction::Store};
  std::array<double, 4> clearValue{0.0, 0.0, 0.0, 0.0};
};

class DepthAttachment {
public:
  const Texture *texture{nullptr};
  LoadAction load{LoadAction::DontCare};
  StoreAction store{StoreAction::DontCare};
  double clearDepth{1.0};
  std::uint8_t clearStencil{0};
};

class RenderPassDescription {
public:
  std::vector<ColorAttachment> colors;
  DepthAttachment depth;
  std::uint32_t x{0}, y{0};
  std::uint32_t width{0}, height{0};
  const char *debugName{nullptr};
};

class RenderPass {
public:
  RenderPass(Device &device, const RenderPassDescription &description);
  ~RenderPass();

  RenderPass(const RenderPass &) = delete;
  RenderPass &operator=(const RenderPass &) = delete;

  [[nodiscard]] const RenderPassDescription &Description() const;
  [[nodiscard]] RenderPassImpl &GetImpl();

private:
  std::unique_ptr<RenderPassImpl> m_impl;
};

} // namespace engine::rhi

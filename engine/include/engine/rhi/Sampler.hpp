#pragma once

#include "engine/rhi/Enums.hpp"

#include <memory>

namespace engine::rhi {

class Device;
class SamplerImpl;

class SamplerCreateInfo {
public:
  Filter minFilter{Filter::Linear};
  Filter magFilter{Filter::Linear};
  Filter mipmapMode{Filter::Linear};
  AddressMode wrapU{AddressMode::ClampToEdge};
  AddressMode wrapV{AddressMode::ClampToEdge};
  float maxAnisotropy{1.0f};
  const char *debugName{nullptr};
};

class Sampler {
public:
  Sampler(Device &device, const SamplerCreateInfo &createInfo);
  ~Sampler();

  Sampler(const Sampler &) = delete;
  Sampler &operator=(const Sampler &) = delete;

  [[nodiscard]] SamplerImpl &GetImpl();

private:
  std::unique_ptr<SamplerImpl> m_impl;
};

} // namespace engine::rhi

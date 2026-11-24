#pragma once

#include "engine/rhi/Enums.hpp"
#include <cstdint>
#include <memory>
#include <vector>
namespace engine::rhi {

class Device;
class BindGroupLayoutImpl;

enum class BindingType : std::uint8_t {
  UniformBuffer,
  StorageBuffer,
  SampledImage,
};

class BindGroupLayoutBinding {
public:
  std::uint32_t binding{0};
  BindingType type{BindingType::UniformBuffer};
  ShaderStageFlags stages{ShaderStageFlags::None};
  bool dynamicOffset{false};
};

class BindGroupLayoutCreateInfo {
public:
  std::vector<BindGroupLayoutBinding> bindings;
  const char *debugName{nullptr};
};

class BindGroupLayout {
public:
  BindGroupLayout(Device &device, const BindGroupLayoutCreateInfo &createInfo);
  ~BindGroupLayout();

  BindGroupLayout(const BindGroupLayout &) = delete;
  BindGroupLayout &operator=(const BindGroupLayout &) = delete;

  [[nodiscard]] BindGroupLayoutImpl &GetImpl();

private:
  std::unique_ptr<BindGroupLayoutImpl> m_impl;
};

} // namespace engine::rhi

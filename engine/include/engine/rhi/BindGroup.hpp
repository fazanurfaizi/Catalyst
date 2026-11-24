#pragma once

#include "engine/rhi/BindGroupLayout.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace engine::rhi {

class BindGroupImpl;
class Buffer;
class Sampler;
class Texture;

class BindGroupEntry {
public:
  std::uint32_t binding{0};
  const Buffer *buffer{nullptr}; // UniformBuffer / StorageBuffer
  std::size_t offset{0};
  std::size_t size{0};
  const Texture *texture{nullptr}; // SampledImage
  const Sampler *sampler{nullptr};
};

class BindGroupCreateInfo {
public:
  const BindGroupLayout *layout{nullptr};
  std::vector<BindGroupEntry> entries;
  const char *debugName{nullptr};
};

class BindGroup {
public:
  BindGroup(Device &device, const BindGroupCreateInfo &createInfo);
  ~BindGroup();

  BindGroup(const BindGroup &) = delete;
  BindGroup &operator=(const BindGroup &) = delete;

  [[nodiscard]] BindGroupImpl &GetImpl();

private:
  std::unique_ptr<BindGroupImpl> m_impl;
};

} // namespace engine::rhi

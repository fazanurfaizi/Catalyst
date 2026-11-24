#pragma once

#include "engine/rhi/Enums.hpp"
#include <cstddef>
#include <memory>
#include <span>

namespace engine::rhi {

class Device;
class BufferImpl;

class BufferCreateInfo {
public:
  std::size_t capacity{0};
  BufferUsage usage{BufferUsage::None};
  const void *initData{nullptr};
  const char *debugName{nullptr};
};

class Buffer {
public:
  Buffer(Device &device, const BufferCreateInfo &createInfo);
  ~Buffer();

  Buffer(const Buffer &) = delete;
  Buffer &operator=(const Buffer &) = delete;

  void SetData(std::size_t offset, std::size_t size, const void *data);

  // Persistent-mapped path: valid when usage has CpuWritable.
  [[nodiscard]] std::span<std::byte> Map();
  void Flush(std::size_t offset, std::size_t size);

  [[nodiscard]] std::size_t Capacity() const;
  [[nodiscard]] BufferImpl &GetImpl();

private:
  std::unique_ptr<BufferImpl> m_impl;
};

} // namespace engine::rhi

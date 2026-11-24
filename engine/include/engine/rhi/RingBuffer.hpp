#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>

namespace engine::rhi {

class Buffer;
class Device;

class RingBufferCreateInfo {
public:
  std::size_t size{4u * 1024u * 1024u};
  const char *debugName{nullptr};
};

class RingBufferRange {
public:
  RingBufferRange() = default;
  ~RingBufferRange();

  RingBufferRange(const RingBufferRange &) = delete;
  RingBufferRange &operator=(const RingBufferRange &) = delete;

  RingBufferRange(RingBufferRange &&other) noexcept;

  [[nodiscard]] std::span<std::byte> Span() const;
  [[nodiscard]] std::size_t Offset() const;
  void Release();

private:
  friend class RingBuffer;
  class Impl;
  std::unique_ptr<Impl> m_impl;
};

class RingBuffer {
public:
  RingBuffer(Device &device, const RingBufferCreateInfo &createInfo);
  ~RingBuffer();

  [[nodiscard]] RingBufferRange Acquire(std::size_t size, std::size_t alignment);
  [[nodiscard]] Buffer &GetBuffer();
  [[nodiscard]] std::size_t Capacity() const;

  // Called once per completed frame with the frame index that just finished
  void OnFrameCompleted(std::uint64_t frameIndex);

private:
  class Impl;
  std::unique_ptr<Impl> m_impl;
};

} // namespace engine::rhi

#pragma once

#include "engine/rhi/Buffer.hpp"
#include "engine/rhi/Enums.hpp"
#include "engine/rhi/gl/gl_objects.hpp"

#include <cstddef>
#include <cstdint>
#include <span>

namespace engine::rhi {

class BufferImpl final {
public:
  explicit BufferImpl(Device &device);

  BufferImpl(Device &device, const BufferCreateInfo &createInfo) noexcept;
  ~BufferImpl() noexcept;

  BufferImpl(const BufferImpl &) = delete;
  BufferImpl &operator=(const BufferImpl &) = delete;

  BufferImpl(BufferImpl &&other) noexcept;
  BufferImpl &operator=(BufferImpl &&other) noexcept;

  [[nodiscard]] std::size_t CapacityByteCount() const noexcept { return m_capacityByteCount; }

  [[nodiscard]] BufferUsage Usage() const noexcept { return m_usage; }

  [[nodiscard]] BufferStorage Storage() const noexcept { return m_storage; }

  // Buffer device address: Vulkan-only concept, always 0 on GL.
  [[nodiscard]] std::uint64_t DeviceAddress() const noexcept { return 0; }

  // Maps the whole buffer on first use. Offsets are in bytes. Valid until
  // Unmap() (Data storage) or until the buffer is destroyed (PersistentMapped).
  [[nodiscard]] std::span<std::byte> Map() const;
  [[nodiscard]] std::span<std::byte> MapAllBytes() noexcept;
  [[nodiscard]] std::span<std::byte> MapBytes(std::size_t byteOffset,
                                              std::size_t byteCount) noexcept;

  // Releases a map created by MapBytes/MapAllBytes. No-op for a persistent map:
  // that mapping is owned by the buffer for its whole lifetime.
  void Unmap() noexcept;

  // Visibility helpers. Coherent mappings need no flush and no invalidate
  void Invalidate(std::size_t byteOffset, std::size_t byteCount) noexcept;
  void FlushBytes(std::size_t byteOffset, std::size_t byteCount) noexcept;
  void FlushAndUnmapBytes(std::size_t byteCount) noexcept;

  // Writes through the persistent map when the buffer is mapped, otherwise
  // through glNamedBufferSubData.
  void Upload(std::size_t byteOffset, std::size_t byteCount, const void *data) noexcept;

  void Dump() const noexcept;

  std::span<std::byte> BeginWrite(std::size_t byteOffset, std::size_t byteCount) noexcept;
  void EndWrite(std::size_t byteOffset, std::size_t byteCount) noexcept;

  void PublishFromWorker() const noexcept;
  void WaitPublication() const noexcept;
  void PublishForHandoff() const noexcept;

  template <typename T>
  [[nodiscard]] std::span<T> MapElements(const std::size_t elementOffset,
                                         const std::size_t elementCount) noexcept {
    const std::size_t byteOffset = elementOffset * sizeof(T);
    const std::size_t byteCount = elementCount * sizeof(T);
    auto rawMap = MapBytes(byteOffset, byteCount);
    return std::span(reinterpret_cast<T *>(rawMap.data()), rawMap.size_bytes() / sizeof(T));
  }

  [[nodiscard]] unsigned int Name() const noexcept;

private:
  void AllocateStorage(const void *initData = nullptr);
  void EnsureMapped() noexcept;
  [[nodiscard]] bool IsMapped() const noexcept { return m_mappedPointer != nullptr; }

  Device &m_device;
  GLBuffer m_handle;
  std::size_t m_capacityByteCount{0};
  BufferUsage m_usage{BufferUsage::None};
  BufferStorage m_storage{BufferStorage::Data};
  std::byte *m_mappedPointer{nullptr};
};

class BufferImplHash {
public:
  [[nodiscard]] std::size_t operator()(const BufferImpl &buffer) const noexcept {
    return static_cast<std::size_t>(buffer.Name());
  }
};

[[nodiscard]] bool operator==(const BufferImpl &lhs, const BufferImpl &rhs) noexcept;
[[nodiscard]] bool operator!=(const BufferImpl &lhs, const BufferImpl &rhs) noexcept;

} // namespace engine::rhi

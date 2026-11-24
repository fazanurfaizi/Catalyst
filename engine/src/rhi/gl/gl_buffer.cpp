#include "engine/rhi/gl/gl_buffer.hpp"

#include "engine/rhi/Enums.hpp"
#include "engine/utils/Logger.hpp"
#include "glad/gl.h"

#include <cassert>
#include <cstdlib>
#include <cstring>

namespace engine::rhi {

namespace {

[[nodiscard]] GLuint CreateGlName() {
  GLuint name{0};
  glCreateBuffers(1, &name);
  return name;
}

[[nodiscard]] bool HasFlag(const BufferUsage value, const BufferUsage flag) noexcept {
  return (static_cast<std::uint32_t>(value) & static_cast<std::uint32_t>(flag)) != 0u;
}

[[nodiscard]] GLsizeiptr ToGlSize(const std::size_t byteCount) noexcept {
  return static_cast<GLsizeiptr>(byteCount);
}

[[nodiscard]] BufferStorage ToStorage(const BufferUsage usage) noexcept {
  return HasFlag(usage, BufferUsage::CpuWritable) ? BufferStorage::PersistentMapped
                                                  : BufferStorage::Data;
}

[[nodiscard]] GLenum ToDrawHint(const BufferUsage usage) noexcept {
  const bool rewrittenEveryFrame =
      HasFlag(usage, BufferUsage::Uniform) || HasFlag(usage, BufferUsage::Storage);
  return rewrittenEveryFrame ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
}

constexpr GLbitfield kPersistentMapFlags =
    GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
constexpr GLbitfield kOnDemandMapFlags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;

} // namespace

BufferImpl::BufferImpl(Device &device) : m_device(device), m_handle(CreateGlName()) {}

BufferImpl::BufferImpl(Device &device, const BufferCreateInfo &createInfo) noexcept
    : m_device(device), m_handle{CreateGlName()}, m_capacityByteCount(createInfo.capacity),
      m_usage(createInfo.usage), m_storage{ToStorage(createInfo.usage)} {
  if (m_capacityByteCount > 0) {
    AllocateStorage(createInfo.initData);
  }
}

BufferImpl::~BufferImpl() noexcept {
  if (m_mappedPointer != nullptr) {
    glUnmapBuffer(m_handle.GlName());
    m_mappedPointer = nullptr;
  }
}

BufferImpl::BufferImpl(BufferImpl &&other) noexcept
    : m_device(other.m_device), m_handle(std::move(other.m_handle)),
      m_capacityByteCount(other.m_capacityByteCount), m_usage(other.m_usage),
      m_storage(other.m_storage), m_mappedPointer(other.m_mappedPointer) {
  other.m_capacityByteCount = 0;
  other.m_usage = BufferUsage::None;
  other.m_storage = BufferStorage::Data;
  other.m_mappedPointer = nullptr;
}

BufferImpl &BufferImpl::operator=(BufferImpl &&other) noexcept {
  if (this == &other) {
    return *this;
  }

  assert(&m_device == &other.m_device && "moving a BufferImpl across devices is not supported");

  if (m_mappedPointer != nullptr) {
    glUnmapNamedBuffer(m_handle.GlName());
    m_mappedPointer = nullptr;
  }

  m_handle = std::move(other.m_handle);
  m_capacityByteCount = other.m_capacityByteCount;
  m_usage = other.m_usage;
  m_storage = other.m_storage;
  m_mappedPointer = other.m_mappedPointer;

  other.m_capacityByteCount = 0;
  other.m_usage = BufferUsage::None;
  other.m_storage = BufferStorage::Data;
  other.m_mappedPointer = nullptr;

  return *this;
}

void BufferImpl::AllocateStorage(const void *initData) {
  assert(m_capacityByteCount > 0);

  if (m_storage == BufferStorage::PersistentMapped) {
    glNamedBufferStorage(m_handle.GlName(), ToGlSize(m_capacityByteCount), initData,
                         kPersistentMapFlags);

    m_mappedPointer = static_cast<std::byte *>(glMapNamedBufferRange(
        m_handle.GlName(), 0, ToGlSize(m_capacityByteCount), kPersistentMapFlags));
    if (m_mappedPointer == nullptr) {
      Logger::Error("[RHI/GL] glMapNamedBufferRange failed for a %zu byte persistent buffer",
                    m_capacityByteCount);
      std::abort();
    }
    return;
  }

  glNamedBufferData(m_handle.GlName(), ToGlSize(m_capacityByteCount), initData,
                    ToDrawHint(m_usage));
}

void BufferImpl::EnsureMapped() noexcept {
  if (m_mappedPointer != nullptr || m_capacityByteCount == 0) {
    return;
  }

  m_mappedPointer = static_cast<std::byte *>(glMapNamedBufferRange(
      m_handle.GlName(), 0, ToGlSize(m_capacityByteCount),
      m_storage == BufferStorage::PersistentMapped ? kPersistentMapFlags : kOnDemandMapFlags));
  if (m_mappedPointer == nullptr) {
    Logger::Error("[RHI/GL] glMapNamedBufferRange failed for buffer %u (%zu bytes)",
                  m_handle.GlName(), m_capacityByteCount);
    std::abort();
  }
}

std::span<std::byte> BufferImpl::Map() const {
  return const_cast<BufferImpl *>(this)->MapAllBytes();
}

std::span<std::byte> BufferImpl::MapAllBytes() noexcept {
  EnsureMapped();
  return {m_mappedPointer, m_mappedPointer != nullptr ? m_capacityByteCount : 0};
}

std::span<std::byte> BufferImpl::MapBytes(std::size_t byteOffset, std::size_t byteCount) noexcept {
  assert(byteOffset <= m_capacityByteCount);
  assert(byteCount <= m_capacityByteCount - byteOffset);

  EnsureMapped();
  if (m_mappedPointer == nullptr) {
    return {};
  }

  return {m_mappedPointer + byteOffset, byteCount};
}

void BufferImpl::Unmap() noexcept {
  if (m_mappedPointer == nullptr) {
    return;
  }

  // A persistent map is owned by the buffer (RingBuffer writes into it every frame)
  if (m_storage == BufferStorage::PersistentMapped) {
    return;
  }

  glUnmapNamedBuffer(m_handle.GlName());
  m_mappedPointer = nullptr;
}

void BufferImpl::Invalidate([[maybe_unused]] std::size_t byteOffset,
                            [[maybe_unused]] std::size_t byteCount) noexcept {
  // No CPU-bisible GPU writes exist for these buffers.
}

void BufferImpl::FlushBytes(std::size_t byteOffset, std::size_t byteCount) noexcept {
  if (m_mappedPointer == nullptr || m_storage == BufferStorage::PersistentMapped) {
    return;
  }
  glFlushMappedBufferRange(m_handle.GlName(), static_cast<GLintptr>(byteOffset),
                           ToGlSize(byteCount));
}

void BufferImpl::FlushAndUnmapBytes(std::size_t byteCount) noexcept {
  FlushBytes(0, byteCount);
  Unmap();
}

void BufferImpl::Upload(std::size_t byteOffset, std::size_t byteCount, const void *data) noexcept {
  assert(data != nullptr || byteCount == 0);
  assert(byteOffset <= m_capacityByteCount);
  assert(byteCount <= m_capacityByteCount - byteOffset);

  if (byteCount == 0)
    return;

  if (m_storage == BufferStorage::PersistentMapped) {
    assert(m_mappedPointer != nullptr);
    std::memcpy(m_mappedPointer + byteOffset, data, byteCount);
    return;
  }

  glNamedBufferSubData(m_handle.GlName(), static_cast<GLintptr>(byteOffset), ToGlSize(byteCount),
                       data);
}

void BufferImpl::Dump() const noexcept {
  const char *storage = m_storage == BufferStorage::PersistentMapped ? "persistent-mapped" : "data";
  Logger::Info("[RHI/GL] buffer %u: %zu bytes, usage 0x%08x, storage %s, mapped %s",
               m_handle.GlName(), m_capacityByteCount, static_cast<std::uint32_t>(m_usage), storage,
               m_mappedPointer != nullptr ? "Yes" : "No");
}

std::span<std::byte> BufferImpl::BeginWrite(std::size_t byteOffset,
                                            std::size_t byteCount) noexcept {
  return MapBytes(byteOffset, byteCount);
}

void BufferImpl::EndWrite(std::size_t byteOffset, std::size_t byteCount) noexcept {
  FlushBytes(byteOffset, byteCount);
}

void BufferImpl::PublishFromWorker() const noexcept {
  // Single-threaded renderer: no publication handshake needed. The ring buffer
  // is made safe by per-frame fencing instead.
}

void BufferImpl::WaitPublication() const noexcept {}

void BufferImpl::PublishForHandoff() const noexcept {}

unsigned int BufferImpl::Name() const noexcept { return m_handle.GlName(); }

bool operator==(const BufferImpl &lhs, const BufferImpl &rhs) noexcept {
  return lhs.Name() == rhs.Name();
}

bool operator!=(const BufferImpl &lhs, const BufferImpl &rhs) noexcept { return !(lhs == rhs); }

} // namespace engine::rhi

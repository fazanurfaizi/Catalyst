#include "engine/rhi/Buffer.hpp"

#if defined(CATALYST_RHI_GL)
#include "engine/rhi/gl/gl_buffer.hpp"
#elif defined(CATALYST_RHI_VULKAN)
// #include "engine/rhi/vulkan/vulkan_buffer.hpp"
#else
#error "No RHI backend selected: define CATALYST_RHI_GL or CATALYST_RHI_VULKAN"
#endif

namespace engine::rhi {

Buffer::Buffer(Device &device, const BufferCreateInfo &createInfo)
    : m_impl{std::make_unique<BufferImpl>(device, createInfo)} {}

// Out of line so the impl type only has to be complete in this translation
// unit, not in every caller of the header.
Buffer::~Buffer() = default;

void Buffer::SetData(std::size_t offset, std::size_t size, const void *data) {
  m_impl->Upload(offset, size, data);
}

std::span<std::byte> Buffer::Map() { return m_impl->Map(); }

void Buffer::Flush(std::size_t offset, std::size_t size) { m_impl->FlushBytes(offset, size); }

std::size_t Buffer::Capacity() const { return m_impl->CapacityByteCount(); }

BufferImpl &Buffer::GetImpl() { return *m_impl; }

} // namespace engine::rhi

#include "engine/rhi/Texture.hpp"

#if defined(CATALYST_RHI_GL)
#include "engine/rhi/gl/gl_texture.hpp"
#elif defined(CATALYST_RHI_VULKAN)
#include "engine/rhi/gl/vulkan_texture.hpp"
#else
#error "No RHI backend selected"
#endif // defined (CATALYST_RHI_GL)

#include "engine/rhi/Device.hpp"

namespace engine::rhi {

Texture::Texture(Device &device, const TextureCreateInfo &createInfo) : m_impl{std::make_unique<TextureImpl>(device, createInfo)} {}

Texture::~Texture() = default;

void Texture::SetData(const void *data, std::uint32_t size, std::uint32_t level) { m_impl->SetData(data, size, level); }

} // namespace engine::rhi

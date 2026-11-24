#pragma once

#include <cstdint>
#include <string_view>

namespace engine::rhi {

class DeviceInfo {
public:
  std::string_view backendName{};                   // "OpenGL 4.5", "Vulkan 1.3"
  std::uint32_t uniformBufferAlignment{0};          // GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT
  std::uint32_t maxColorAttachments{0};             // GL_MAX_COLOR_ATTACHMENTS
  std::uint32_t uniformBufferBindingAlignment{0};
  bool supportsBaseInstance{false};
  bool supportsDynamicRendering{false};             // Vulkan
};

} // namespace engine::rhi

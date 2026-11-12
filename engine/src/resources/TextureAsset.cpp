#include "engine/resources/TextureAsset.hpp"

namespace engine::resources {

bool TextureAsset::EnsureGpu() const {
  if (m_gpuReady)
    return true;
  if (!HandleGpuUpload())
    return false;
  m_gpuReady = true;

  if (!m_keepCpuData) {
    const_cast<TextureAsset *>(this)->m_cpuPixels.clear();
    const_cast<TextureAsset *>(this)->m_cpuPixels.shrink_to_fit();
  }

  return true;
}

void TextureAsset::ReleaseGpu() const {
  if (!m_gpuReady)
    return;

  m_texture = graphics::Texture{};
  m_gpuReady = false;
}

} // namespace engine::resources

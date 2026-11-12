#include "engine/resources/TextureAtlasAsset.hpp"
#include "engine/utils/Logger.hpp"

namespace engine::resources {

bool TextureAtlasAsset::HandleGpuUpload() const {
  if (m_cpuPixels.empty()) {
    Logger::Error("[TextureAtlasAsset] no CPU pixel data to upload");
    return false;
  }

  m_texture.Upload(static_cast<int>(m_spec.width),
                   static_cast<int>(m_spec.height), ImageFormat::RGBA8,
                   m_cpuPixels.data());
  return m_texture.IsValid();
}

} // namespace engine::resources

#include "engine/resources/TextureAtlasAsset.hpp"
#include "engine/utils/Logger.hpp"
#include "glad/gl.h"

namespace engine::resources {

bool TextureAtlasAsset::HandleGpuUpload() const {
  if (m_cpuPixels.empty()) {
    Logger::Error("[TextureAtlasAsset] no CPU pixel data to upload");
    return false;
  }

  GLint glMax = 0;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMax);

  if (glMax <= 0) {
    Logger::Error("[TextureAtlasAsset] no GL context");
    return false;
  }

  if (static_cast<GLint>(m_spec.width) > glMax ||
      static_cast<GLint>(m_spec.height) > glMax) {
    Logger::Error(
        "[TextureAtlasAsset] atlas %ux%u exceeds GL_MAX_TEXTURE_SIZE %d",
        m_spec.width, m_spec.height, glMax);
    return false;
  }

  m_texture.Upload(static_cast<int>(m_spec.width),
                   static_cast<int>(m_spec.height), ImageFormat::RGBA8,
                   m_cpuPixels.data());
  return m_texture.IsValid();
}

} // namespace engine::resources

#include "engine/resources/Texture2DAsset.hpp"
#include "engine/utils/Logger.hpp"

namespace engine::resources {

Texture2DAsset::Texture2DAsset(asset::AssetHandle handle)
    : TextureAsset(handle) {}

std::shared_ptr<Texture2DAsset>
Texture2DAsset::Create(asset::AssetHandle handle,
                       const TextureSpecification &spec, const Buffer &data,
                       bool uploadImmediately) {
  if (data.data == nullptr || data.size == 0) {
    Logger::Error("[Texture2DAsset] Create: empty buffer");
    return nullptr;
  }
  if (spec.width == 0 || spec.height == 0) {
    Logger::Error("[Texture2DAsset] Create: zero-sized texture");
    return nullptr;
  }

  auto asset = std::make_shared<Texture2DAsset>(handle);
  asset->m_spec = spec;
  asset->m_cpuPixels.assign(data.data, data.data + data.size);

  if (uploadImmediately) {
    if (!asset->EnsureGpu()) {
      Logger::Error("[Texture2DAsset] eager upload failed for handle %s",
                    handle.ToString().c_str());
      return nullptr;
    }
  }

  return asset;
}

void Texture2DAsset::SetData(Buffer data) {
  if (!data.data || data.size == 0)
    return;

  ReleaseGpu();
  m_cpuPixels.clear();
  if (data.data && data.size > 0) {
    m_cpuPixels.assign(data.data, data.data + data.size);
  }
}

bool Texture2DAsset::HandleGpuUpload() const {
  if (m_cpuPixels.empty()) {
    Logger::Error("[Texture2DAsset] no CPU pixel data to upload");
    return false;
  }
  m_texture.Upload(static_cast<int>(m_spec.width),
                   static_cast<int>(m_spec.height), ImageFormat::RGBA8,
                   (m_cpuPixels.data()));

  return m_texture.IsValid();
}
} // namespace engine::resources

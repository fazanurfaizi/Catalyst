#include "engine/resources/Texture2DAsset.hpp"
#include "engine/utils/Logger.hpp"

namespace engine::resources {

Texture2DAsset::Texture2DAsset(asset::AssetHandle handle)
    : TextureAsset(handle) {}

std::shared_ptr<Texture2DAsset>
Texture2DAsset::Create(asset::AssetHandle handle,
                       const TextureSpecification &spec, const Buffer &data) {
  if (data.data == nullptr || data.size == 0) {
    Logger::Error("[Texture2DAsset] Create: empty buffer");
    return nullptr;
  }
  if (spec.width == 0 || spec.height == 0) {
    Logger::Error("[Texture2DAsset] Create: zero-sized texture");
    return nullptr;
  }

  auto tex = std::make_shared<Texture2DAsset>(handle);
  tex->m_spec = spec;
  tex->m_texture.Upload(
      static_cast<int>(spec.width), static_cast<int>(spec.height), spec.format,
      static_cast<const std::uint8_t *>(data.data), /*generateMipmaps=*/true);

  if (!tex->m_texture.IsValid()) {
    Logger::Error("[Texture2DAsset] Create: GL upload failed");
    return nullptr;
  }
  return tex;
}

void Texture2DAsset::SetData(Buffer data) {
  if (!data.data || data.size == 0)
    return;
  m_texture.Upload(static_cast<int>(m_spec.width),
                   static_cast<int>(m_spec.height), m_spec.format,
                   static_cast<const std::uint8_t *>(data.data),
                   /*generateMipmaps=*/true);
}

} // namespace engine::resources

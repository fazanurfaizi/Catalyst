#pragma once

#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetType.hpp"
#include "engine/graphics/assets/TextureAsset.hpp"
#include "engine/graphics/texture/Texture.hpp"
#include "engine/utils/Buffer.hpp"
#include <memory>

namespace engine::graphics {

class Texture2DAsset : public TextureAsset {
public:
  explicit Texture2DAsset(asset::AssetHandle handle);

  static asset::AssetType StaticType() { return asset::AssetType::Texture2D; }
  asset::AssetType Type() const override { return StaticType(); }

  static std::shared_ptr<Texture2DAsset>
  Create(asset::AssetHandle handle, const TextureSpecification &spec,
         const Buffer &data);

  const TextureSpecification &GetSpecification() const override {
    return m_spec;
  }

  uint32_t GetWidth() const override { return m_texture.Width(); }

  uint32_t GetHeight() const override { return m_texture.Height(); }

  uint32_t GetRendererID() const override { return m_texture.Id(); }

  void SetData(Buffer data) override;

  void Bind(uint32_t slot = 0) const override { m_texture.Bind(slot); }

  bool operator==(const TextureAsset &other) const override {
    return GetRendererID() == other.GetRendererID();
  }

  bool equals(const TextureAsset &other) const override {
    return GetRendererID() == other.GetRendererID();
  }

private:
  TextureSpecification m_spec;
  Texture m_texture;
};

} // namespace engine::graphics

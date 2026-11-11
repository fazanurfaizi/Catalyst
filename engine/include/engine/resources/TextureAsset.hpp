#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/graphics/texture/Texture.hpp"
#include "engine/resources/TextureFormat.hpp"
#include "engine/utils/Buffer.hpp"

namespace engine::resources {

class TextureAsset : public asset::Asset {
public:
  virtual ~TextureAsset() = default;

  virtual const TextureSpecification &GetSpecification() const = 0;

  virtual uint32_t GetWidth() const { return m_texture.Width(); };
  virtual uint32_t GetHeight() const { return m_texture.Height(); };
  virtual uint32_t GetRendererID() const { return m_texture.Id(); };

  virtual void SetData(Buffer data) {};

  virtual void Bind(uint32_t slot = 0) const = 0;

  virtual bool operator==(const TextureAsset &other) const {
    return GetRendererID() == other.GetRendererID();
  };
  virtual bool equals(const TextureAsset &other) const {
    return GetRendererID() == other.GetRendererID();
  };

protected:
  explicit TextureAsset(asset::AssetHandle handle) : asset::Asset(handle) {}

  TextureSpecification m_spec;

  graphics::Texture m_texture;
};

} // namespace engine::resources

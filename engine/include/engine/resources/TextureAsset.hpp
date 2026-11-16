#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/graphics/texture/Texture.hpp"
#include "engine/resources/TextureFormat.hpp"
#include <cstdint>

namespace engine::resources {

class TextureAsset : public asset::Asset {
public:
  virtual ~TextureAsset() = default;

  virtual const TextureSpecification &GetSpecification() const {
    return m_spec;
  }

  virtual uint32_t GetWidth() const { return m_texture.Width(); };
  virtual uint32_t GetHeight() const { return m_texture.Height(); };

  bool EnsureGpu() const override;
  bool IsGpuReady() const override { return m_gpuReady; }
  void ReleaseGpu() const override;

  std::uint32_t GetRendererID() const {
    if (!EnsureGpu())
      return 0;
    return m_texture.Id();
  }

  void Bind(std::uint32_t slot = 0) const {
    if (!EnsureGpu())
      return;
    m_texture.Bind(slot);
  }

  void SetKeepCpuData(bool keep) noexcept { m_keepCpuData = keep; }
  bool KeepsCpuData() const noexcept { return m_keepCpuData; }
  bool HasCpuData() const noexcept { return m_cpuPixels.size() > 0; }

  bool operator==(const TextureAsset &other) const {
    return GetRendererID() == other.GetRendererID();
  };
  bool equals(const TextureAsset &other) const {
    return GetRendererID() == other.GetRendererID();
  };

protected:
  explicit TextureAsset(asset::AssetHandle handle) : asset::Asset(handle) {}

  virtual bool HandleGpuUpload() const = 0;

  TextureSpecification m_spec;

  mutable graphics::Texture m_texture;
  mutable bool m_gpuReady = false;
  // mutable Buffer m_cpuPixels;
  mutable std::vector<std::uint8_t> m_cpuPixels;

  bool m_keepCpuData = false;
};

} // namespace engine::resources

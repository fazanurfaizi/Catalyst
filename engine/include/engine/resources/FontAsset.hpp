#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetType.hpp"
#include "engine/asset/importer/font/FontImportResult.hpp"
#include "engine/graphics/text/Font.hpp"

#include <cstdint>
#include <memory>

namespace engine::resources {

class FontAsset : public asset::Asset {
public:
  explicit FontAsset(asset::AssetHandle handle) : asset::Asset(handle) {}

  static asset::AssetType StaticType() { return asset::AssetType::Font; }
  asset::AssetType Type() const override { return StaticType(); }

  std::shared_ptr<const asset::FontImportResult> cpu;

  bool EnsureGpu() const override;
  bool IsGpuReady() const override { return m_gpu != nullptr; }
  void ReleaseGpu() const override;

  std::uint32_t TextHeight() const noexcept {
    return cpu ? cpu->textHeight : 0;
  }

  bool HasCpuData() const noexcept { return cpu != nullptr; }

  const graphics::Font *GpuFont() const noexcept {
    return m_gpu ? m_gpu.get() : nullptr;
  }

private:
  mutable std::shared_ptr<graphics::Font> m_gpu;
};

} // namespace engine::resources

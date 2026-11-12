#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/asset/importer/shader/ShaderImportResult.hpp"
#include "engine/graphics/gl/Shader.hpp"
#include <memory>

namespace engine::resources {

class ShaderAsset : public asset::Asset {
public:
  explicit ShaderAsset(asset::AssetHandle handle) : asset::Asset(handle) {}

  asset::AssetType Type() const override { return StaticType(); }
  static asset::AssetType StaticType() { return asset::AssetType::Shader; }

  std::shared_ptr<const asset::ShaderImportResult> cpu;

  bool EnsureGpu() const override;
  bool IsGpuReady() const override { return m_gpu != nullptr; }
  void ReleaseGpu() const override;

  graphics::Shader *Gpu() const noexcept {
    return m_gpu ? m_gpu.get() : nullptr;
  }

  const char *DebugName() const noexcept {
    return cpu ? cpu->debugName.c_str() : "Unknown";
  }

private:
  mutable std::shared_ptr<graphics::Shader> m_gpu;
};

} // namespace engine::resources

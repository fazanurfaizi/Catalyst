#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"

#include <memory>
#include <span>
#include <string>
#include <string_view>

namespace engine::resources {
class ShaderAsset;
}

namespace engine::asset {

class IAssetManager;

namespace builtin {

inline constexpr std::string_view kSpriteShader = "shader_sprite";
inline constexpr std::string_view kLightShader = "shader_light";
inline constexpr std::string_view kCompositeShader = "shader_composite";
inline constexpr std::string_view kTextShader = "shader_text";

struct ShaderDef {
  std::string_view name;
  std::string_view key;
};

std::span<const ShaderDef> ShaderDefs();

AssetHandle HandleOf(std::string_view name);

std::string PathOf(std::string_view name);

std::shared_ptr<Asset> CreateAsset(AssetHandle handle, std::string_view name);

std::shared_ptr<resources::ShaderAsset> AcquireShader(IAssetManager &assets,
                                                      std::string_view name);

} // namespace builtin

} // namespace engine::asset

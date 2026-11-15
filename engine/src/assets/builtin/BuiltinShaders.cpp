#include "engine/asset/builtin/BuiltinShaders.hpp"

#include "engine/asset/importer/shader/ShaderImportResult.hpp"
#include "engine/asset/manager/IAssetManager.hpp"
#include "engine/resources/ShaderAsset.hpp"
#include "engine/utils/Logger.hpp"

#include <memory>
#include <string>

namespace engine::asset::builtin {

namespace {

struct EmbeddedFile {
  std::string_view path;
  std::string_view source;
};

constexpr EmbeddedFile kEmbeddedFiles[] = {
#include "EmbedShaders.inc"
};

std::string_view FindSource(std::string_view path) {
  for (const auto &f : kEmbeddedFiles) {
    if (f.path == path)
      return f.source;
  }
  return {};
}

constexpr ShaderDef kShaderDefs[] = {
    {kSpriteShader, "sprite"},
    {kLightShader, "light"},
    {kCompositeShader, "composite"},
    {kTextShader, "text"},
};

const ShaderDef *FindDef(std::string_view name) {
  for (const auto &def : kShaderDefs) {
    if (def.name == name)
      return &def;
  }
  return nullptr;
}

} // namespace

std::span<const ShaderDef> ShaderDefs() {
  return std::span<const ShaderDef>(kShaderDefs);
}

std::string PathOf(std::string_view name) {
  const ShaderDef *def = FindDef(name);
  if (!def) {
    return {};
  }
  return "@engine/shaders/" + std::string(def->key);
}

AssetHandle HandleOf(std::string_view name) {
  const std::string path = PathOf(name);
  return path.empty() ? AssetHandle::Null()
                      : AssetHandle::FromStableString(path);
}

std::shared_ptr<Asset> CreateAsset(AssetHandle handle, std::string_view name) {
  const ShaderDef *def = FindDef(name);
  if (!def) {
    Logger::Error("[BuiltinShaders] unknown shader '%s'",
                  std::string(name).c_str());
    return nullptr;
  }

  const std::string key(def->key);
  const std::string_view vert = FindSource(key + "/" + key + ".vert.glsl");
  const std::string_view frag = FindSource(key + "/" + key + ".frag.glsl");

  if (vert.empty() || frag.empty()) {
    Logger::Error("[BuiltinShaders] embedded source missing for '%s'",
                  key.c_str());
    return nullptr;
  }

  auto cpu = std::make_shared<ShaderImportResult>();
  cpu->vertexSource.assign(vert);
  cpu->fragmentSource.assign(frag);
  cpu->debugName = "builtin/" + key;

  auto asset = std::make_shared<resources::ShaderAsset>(handle);
  asset->cpu = std::move(cpu);
  return asset;
}

std::shared_ptr<resources::ShaderAsset> AcquireShader(IAssetManager &assets,
                                                      std::string_view name) {
  if (auto shader = assets.AcquireByName<resources::ShaderAsset>(name))
    return shader;

  return assets.Acquire<resources::ShaderAsset>(HandleOf(name));
}

} // namespace engine::asset::builtin

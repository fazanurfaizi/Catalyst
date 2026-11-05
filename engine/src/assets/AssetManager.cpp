#include "engine/assets/AssetManager.hpp"
#include "engine/assets/AssetPath.hpp"
#include "engine/graphics/gl/Shader.hpp"
#include "engine/graphics/text/Font.hpp"
#include "engine/graphics/texture/Texture.hpp"
#include "engine/utils/Logger.hpp"
#include <memory>

namespace engine::assets {

AssetManager::AssetManager() = default;
AssetManager::~AssetManager() = default;
AssetManager::AssetManager(AssetManager &&) noexcept = default;
AssetManager &AssetManager::operator=(AssetManager &&) noexcept = default;

graphics::Texture *AssetManager::LoadTexture(const std::string &key,
                                             const std::string &relativePath) {
  auto it = m_textures.find(key);
  if (it != m_textures.end()) {
    return it->second.get();
  }

  auto texture = std::make_unique<graphics::Texture>();
  if (!texture->LoadFromFile(GetAssetPath(relativePath))) {
    Logger::Error("[AssetManager] Failed to load texture: %s",
                  relativePath.c_str());
    return nullptr;
  }

  auto *ptr = texture.get();
  m_textures[key] = std::move(texture);
  return ptr;
}

graphics::Texture *AssetManager::GetTexture(const std::string &key) const {
  auto it = m_textures.find(key);
  return it != m_textures.end() ? it->second.get() : nullptr;
}

graphics::Shader *AssetManager::LoadShader(const std::string &key,
                                           const std::string &relativePath) {

  auto it = m_shaders.find(key);
  if (it != m_shaders.end()) {
    return it->second.get();
  }

  auto shader = std::make_unique<graphics::Shader>();
  if (!shader->LoadFromFile(GetAssetPath(relativePath))) {
    Logger::Error("[AssetManager] shader '%s' failed to load (%s)", key.c_str(),
                  relativePath.c_str());
    return nullptr;
  }

  auto *ptr = shader.get();
  m_shaders[key] = std::move(shader);
  return ptr;
}

graphics::Shader *AssetManager::GetShader(const std::string &key) const {
  auto it = m_shaders.find(key);
  return it != m_shaders.end() ? it->second.get() : nullptr;
}

graphics::Font *AssetManager::LoadFont(const std::string &key,
                                       const std::string &relativePath,
                                       float fontSize) {
  auto it = m_fonts.find(key);
  if (it != m_fonts.end()) {
    return it->second.get();
  }

  auto font = std::make_unique<graphics::Font>();
  if (!font->LoadFromFile(GetAssetPath(relativePath), fontSize)) {
    Logger::Error("[AssetManager] Failed to load font: %s",
                  relativePath.c_str());
    return nullptr;
  }

  auto *ptr = font.get();
  m_fonts[key] = std::move(font);
  return ptr;
}

graphics::Font *AssetManager::GetFont(const std::string &key) const {
  auto it = m_fonts.find(key);
  return it != m_fonts.end() ? it->second.get() : nullptr;
}

std::string AssetManager::GetAssetPath(const std::string &relativePath) const {
  return AssetPath(relativePath);
}

void AssetManager::Clear() {
  m_textures.clear();
  m_fonts.clear();
  m_shaders.clear();
}

} // namespace engine::assets

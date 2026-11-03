#include "engine/resources/ResourceManager.hpp"
#include "engine/assets/AssetPath.hpp"
#include "engine/graphics/Font.hpp"
#include "engine/graphics/Shader.hpp"
#include "engine/graphics/Texture.hpp"
#include "engine/utils/Logger.hpp"
#include <memory>

namespace engine::resources {

ResourceManager::ResourceManager() = default;
ResourceManager::~ResourceManager() = default;
ResourceManager::ResourceManager(ResourceManager &&) noexcept = default;
ResourceManager &
ResourceManager::operator=(ResourceManager &&) noexcept = default;

graphics::Texture *
ResourceManager::LoadTexture(const std::string &key,
                             const std::string &relativePath) {
  auto it = m_textures.find(key);
  if (it != m_textures.end()) {
    return it->second.get();
  }

  auto texture = std::make_unique<graphics::Texture>();
  if (!texture->LoadFromFile(GetAssetPath(relativePath))) {
    Logger::Error("[ResourceManager] Failed to load texture: %s",
                  relativePath.c_str());
    return nullptr;
  }

  auto *ptr = texture.get();
  m_textures[key] = std::move(texture);
  return ptr;
}

graphics::Texture *ResourceManager::GetTexture(const std::string &key) {
  auto it = m_textures.find(key);
  return it != m_textures.end() ? it->second.get() : nullptr;
}

graphics::Shader *ResourceManager::LoadShader(const std::string &key,
                                              const std::string &relativePath) {

  auto it = m_shaders.find(key);
  if (it != m_shaders.end()) {
    return it->second.get();
  }

  auto shader = std::make_unique<graphics::Shader>();
  if (!shader->LoadFromFile(GetAssetPath(relativePath))) {
    Logger::Error("[ResourceManager] shader '%s' failed to load (%s)",
                  key.c_str(), relativePath.c_str());
    return nullptr;
  }

  auto *ptr = shader.get();
  m_shaders[key] = std::move(shader);
  return ptr;
}

graphics::Shader *ResourceManager::GetShader(const std::string &key) {
  auto it = m_shaders.find(key);
  return it != m_shaders.end() ? it->second.get() : nullptr;
}

graphics::Font *ResourceManager::LoadFont(const std::string &key,
                                          const std::string &relativePath,
                                          float fontSize) {
  auto it = m_fonts.find(key);
  if (it != m_fonts.end()) {
    return it->second.get();
  }

  auto font = std::make_unique<graphics::Font>();
  if (!font->LoadFromFile(GetAssetPath(relativePath), fontSize)) {
    Logger::Error("[ResourceManager] Failed to load font: %s",
                  relativePath.c_str());
    return nullptr;
  }

  auto *ptr = font.get();
  m_fonts[key] = std::move(font);
  return ptr;
}

graphics::Font *ResourceManager::GetFont(const std::string &key) {
  auto it = m_fonts.find(key);
  return it != m_fonts.end() ? it->second.get() : nullptr;
}

std::string
ResourceManager::GetAssetPath(const std::string &relativePath) const {
  return AssetPath(relativePath);
}

void ResourceManager::Clear() {
  m_textures.clear();
  m_fonts.clear();
}

} // namespace engine::resources

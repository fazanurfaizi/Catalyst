#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace engine::graphics {

class Texture;
class Font;
class Shader;

} // namespace engine::graphics

namespace engine::resources {

class ResourceManager {
public:
  ResourceManager();

  ~ResourceManager();

  ResourceManager(const ResourceManager &) = delete;
  ResourceManager &operator=(const ResourceManager &) = delete;

  ResourceManager(ResourceManager &&) noexcept;
  ResourceManager &operator=(ResourceManager &&) noexcept;

  graphics::Texture *LoadTexture(const std::string &key,
                                 const std::string &relativePath);
  graphics::Texture *GetTexture(const std::string &key);

  graphics::Shader *LoadShader(const std::string &key,
                               const std::string &relativePath);

  graphics::Shader *GetShader(const std::string &key);

  graphics::Font *LoadFont(const std::string &key,
                           const std::string &relativePath, float fontSize);
  graphics::Font *GetFont(const std::string &key);

  std::string GetAssetPath(const std::string &relativePath) const;

  void Clear();

private:
  std::unordered_map<std::string, std::unique_ptr<graphics::Texture>>
      m_textures;
  std::unordered_map<std::string, std::unique_ptr<graphics::Font>> m_fonts;
  std::unordered_map<std::string, std::unique_ptr<graphics::Shader>> m_shaders;
};

} // namespace engine::resources

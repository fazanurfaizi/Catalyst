#pragma once

#include "engine/graphics/Texture.hpp"

#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine {

struct AtlasRegion {
  glm::vec2 uvMin{0.0f, 0.0f};
  glm::vec2 uvMax{1.0f, 1.0f};
};

class TextureAtlas {
public:
  TextureAtlas() = default;

  TextureAtlas(const TextureAtlas &) = delete;
  TextureAtlas &operator=(const TextureAtlas &) = delete;

  TextureAtlas &operator=(TextureAtlas &&) noexcept = default;
  TextureAtlas(TextureAtlas &&) noexcept = default;

  int AddFromDirectory(const std::string &directory);
  bool AddFromFile(const std::string &name, const std::string &path);
  bool AddFromPixels(const std::string &name, int w, int h,
                     const std::uint8_t *rgba8);
  bool AddSolid(const std::string &name, std::uint32_t rgba, int size = 16);
  bool AddCheckerboard(const std::string &name, int cells, std::uint32_t a,
                       std::uint32_t b);

  bool Pack();

  const AtlasRegion *GetRegion(const std::string &name) const;
  const Texture &GetTexture() const { return m_texture; }
  bool IsPacked() const { return m_texture.IsValid(); }
  int SourceCount() const { return static_cast<int>(m_sources.size()); }
  bool HasSource(const std::string &name) const; // pre-pack
  bool HasRegion(const std::string &name) const; // post-pack

  std::vector<std::string> RegionNames() const;
  std::vector<std::string> SourceNames() const;

private:
  struct Source {
    std::string name;
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> rgba8;
  };

  std::vector<Source> m_sources;
  std::unordered_map<std::string, AtlasRegion> m_regions;
  Texture m_texture;
};

} // namespace engine

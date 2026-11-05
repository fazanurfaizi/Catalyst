#pragma once

#include "engine/graphics/texture/Texture.hpp"

#include <functional>
#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace engine::graphics {

struct AtlasRegion {
  glm::vec2 uvMin{0.0f, 0.0f};
  glm::vec2 uvMax{1.0f, 1.0f};
};

struct StringHash {
  using is_transparent = void;
  std::size_t operator()(std::string_view sv) const noexcept {
    return std::hash<std::string_view>{}(sv);
  }
  std::size_t operator()(const std::string &s) const noexcept {
    return std::hash<std::string_view>{}(s);
  }
  std::size_t operator()(const char *s) const noexcept {
    return std::hash<std::string_view>{}(s);
  }
};

constexpr std::uint64_t HashName(std::string_view sv) noexcept {
  std::uint64_t h = 1469598103934665603ULL;
  for (unsigned char c : sv) {
    h ^= c;
    h *= 1099511628211ULL;
  }
  return h;
}

class TextureAtlas {
public:
  using RegionId = std::uint16_t;
  static constexpr RegionId kInvalidRegionId = static_cast<RegionId>(-1);

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

  const AtlasRegion *GetRegionById(RegionId id) const;
  std::string_view GetRegionNameById(RegionId id) const;

  const AtlasRegion *GetRegion(const std::string &name) const;
  RegionId TryGetRegionId(std::string_view name) const;

  const Texture &GetTexture() const { return m_texture; }
  bool IsPacked() const { return m_texture.IsValid(); }

  int SourceCount() const { return static_cast<int>(m_sources.size()); }

  bool HasSource(const std::string_view name) const; // pre-pack
  bool HasRegion(const std::string_view name) const; // post-pack

  std::vector<std::string> RegionNames() const;
  std::vector<std::string> SourceNames() const;

  bool IsValid(RegionId id) const { return id != kInvalidRegionId; }

private:
  struct Source {
    std::string name;
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> rgba8;
  };

  void AddSource(Source &&src);

  std::vector<Source> m_sources;

  std::unordered_set<std::string, StringHash, std::equal_to<>> m_sourceNames;

  std::vector<AtlasRegion> m_regionsById;
  std::vector<std::string> m_regionNamesById;
  std::unordered_map<std::string, std::uint16_t, StringHash, std::equal_to<>>
      m_regionIds;

  Texture m_texture;
};

} // namespace engine::graphics

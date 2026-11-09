#pragma once

#include "engine/graphics/assets/TextureAtlasAsset.hpp"
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

#include <vector>

namespace engine::asset {

class TextureAtlasBuilder {
public:
  struct Packed {
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> rgba8;
    std::vector<std::pair<std::string, graphics::AtlasRegion>> regions;
  };

  int AddFromDirectory(const std::string &directory);
  bool AddFromFile(const std::string &name, const std::string &path,
                   int maxDimension = 512);
  bool AddFromPixels(const std::string &name, int w, int h,
                     const std::uint8_t *rgba8);
  bool AddSolid(const std::string &name, std::uint32_t rgba, int size = 16);
  bool AddCheckerboard(const std::string &name, int cells, std::uint32_t a,
                       std::uint32_t b);

  int SourceCount() const { return static_cast<int>(m_sources.size()); }

  bool Pack(Packed &out, int maxSize = 4096) const;

  bool HasSource(const std::string_view name) const {
    return m_sourceNames.contains(std::string(name));
  }

private:
  struct Source {
    std::string name;
    int width;
    int height;
    std::vector<std::uint8_t> rgba8;
  };

  void AddSource(Source &&src);

  std::vector<Source> m_sources;
  std::unordered_set<std::string, graphics::StringHash, std::equal_to<>>
      m_sourceNames;
};

} // namespace engine::asset

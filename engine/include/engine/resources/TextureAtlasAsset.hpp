#pragma once

#include "engine/graphics/texture/Texture.hpp"
#include "engine/resources/TextureAsset.hpp"
#include "engine/utils/StringHash.hpp"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace engine::resources {

struct AtlasRegion {
  glm::vec2 uvMin{0.0f, 0.0f};
  glm::vec2 uvMax{1.0f, 1.0f};
};

class TextureAtlasAsset : public TextureAsset {
public:
  using RegionId = std::uint16_t;
  static constexpr RegionId kInvalidRegionId =
      std::numeric_limits<RegionId>::max();

  explicit TextureAtlasAsset(asset::AssetHandle handle)
      : TextureAsset(handle) {}

  static asset::AssetType StaticType() {
    return asset::AssetType::TextureAtlas;
  }
  asset::AssetType Type() const override { return StaticType(); }

  const AtlasRegion *GetRegion(const std::string &name) const {
    auto it = m_nameToId.find(name);
    return it == m_nameToId.end() ? nullptr : &m_regionsById[it->second];
  }

  RegionId GetRegionId(std::string_view name) const {
    auto it = m_nameToId.find(name);
    return it == m_nameToId.end() ? kInvalidRegionId : it->second;
  }

  const AtlasRegion *GetRegionById(RegionId id) const {
    return IsValid(id) ? &m_regionsById[id] : nullptr;
  }

  std::string_view GetRegionNameById(RegionId id) const {
    return IsValid(id) ? std::string_view(m_regionNamesById[id])
                       : std::string_view{};
  }

  bool IsValid(RegionId id) const { return id < m_regionsById.size(); }

  std::size_t RegionCount() const { return m_regionsById.size(); }

  const graphics::Texture &GetTexture() const { return m_texture; }

  const TextureSpecification &GetSpecification() const override {
    return m_spec;
  }

  void Bind(uint32_t slot = 0) const override { m_texture.Bind(slot); }

  void AssignRegion(std::string name, AtlasRegion region) {
    const RegionId id = static_cast<RegionId>(m_regionsById.size());
    m_regionsById.push_back(region);
    m_regionNamesById.emplace_back(name);
    m_nameToId.emplace(std::move(name), id);
  }

  void AssignTexture(graphics::Texture texture) {
    m_texture = std::move(texture);
    m_spec.width = m_texture.Width();
    m_spec.height = m_texture.Height();
    m_spec.format = ImageFormat::RGBA8;
    m_spec.magFilterLinear = false;
  }

private:
  std::vector<AtlasRegion> m_regionsById;
  std::vector<std::string> m_regionNamesById;
  std::unordered_map<std::string, RegionId, StringHash, std::equal_to<>>
      m_nameToId;
};

} // namespace engine::resources

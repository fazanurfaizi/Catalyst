#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

namespace engine::graphics {

struct PointLightGpu;

class LightTileCuller {
public:
  static constexpr std::uint32_t kTileSizePx = 64;

  void SetTileGrid(std::uint32_t tilesX, std::uint32_t tilesY, float tileWorldW,
                   float tileWorldH, const glm::vec2 &worldOrigin);
  void Build(const PointLightGpu *lights, int count);

  std::uint32_t TilesX() const { return m_tilesX; }
  std::uint32_t TilesY() const { return m_tilesY; }
  float TileWorldW() const { return m_tileW; }
  float TileWorldH() const { return m_tileH; }
  glm::vec2 WorldOrigin() const { return m_origin; }

  const std::vector<std::uint32_t> &Offsets() const { return m_offsets; }
  const std::vector<std::uint32_t> &Indices() const { return m_indices; }

  // Solve 1/(c + l*d + q*d^2) = threshold for d; returns the positive root.
  static float EffectiveRadius(const glm::vec4 &atten, float threshold = 0.02f);

private:
  std::uint32_t m_tilesX = 0;
  std::uint32_t m_tilesY = 0;
  float m_tileW = 1.0f;
  float m_tileH = 1.0f;
  glm::vec2 m_origin{0.0f};
  std::vector<std::uint32_t> m_offsets;
  std::vector<std::uint32_t> m_indices;
  std::vector<std::uint32_t> m_cursor;
};

} // namespace engine::graphics

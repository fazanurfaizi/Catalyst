#include "engine/graphics/LightTileCuller.hpp"
#include "engine/graphics/LightBuffer.hpp"

#include <algorithm>
#include <cmath>

namespace engine::graphics {

float LightTileCuller::EffectiveRadius(const glm::vec4 &atten,
                                       float threshold) {
  // atten = (constant, linear, quadratic, unused)
  const float c = atten.x;
  const float l = atten.y;
  const float q = atten.z;
  if (q <= 1e-6f)
    return 1e4f;
  const float target = 1.0f / std::max(threshold, 1e-4f);
  const float a = q;
  const float b = l;
  const float cc = c - target;
  const float disc = b * b - 4.0f * a * cc;
  if (disc < 0.0f)
    return 0.0f;
  return (-b + std::sqrt(disc)) / (2.0f * a);
}

void LightTileCuller::SetTileGrid(std::uint32_t tilesX, std::uint32_t tilesY,
                                  float tileWorldW, float tileWorldH,
                                  const glm::vec2 &worldOrigin) {
  m_tilesX = std::max(1u, tilesX);
  m_tilesY = std::max(1u, tilesY);
  m_tileW = tileWorldW > 0 ? tileWorldW : 1.0f;
  m_tileH = tileWorldH > 0 ? tileWorldH : 1.0f;
  m_origin = worldOrigin;
}

void LightTileCuller::Build(const PointLightGpu *lights, int count) {
  const std::size_t tileCount = m_tilesX * m_tilesY;
  m_offsets.assign(tileCount + 1, 0);
  m_indices.clear();

  for (int i = 0; i < count; ++i) {
    const auto &L = lights[i];
    const float R = EffectiveRadius(L.attenuation);
    if (R <= 0.0f)
      continue;
    const float px = L.position.x;
    const float py = L.position.y;
    const int x0 = std::max(
        0, static_cast<int>(std::floor((px - R - m_origin.x) / m_tileW)));
    const int x1 = std::min(
        static_cast<int>(m_tilesX) - 1,
        static_cast<int>(std::floor((px + R - m_origin.x) / m_tileW)));
    const int y0 = std::max(
        0, static_cast<int>(std::floor((py - R - m_origin.y) / m_tileH)));
    const int y1 = std::min(
        static_cast<int>(m_tilesY) - 1,
        static_cast<int>(std::floor((py + R - m_origin.y) / m_tileH)));
    for (int y = y0; y <= y1; ++y) {
      for (int x = x0; x <= x1; ++x) {
        ++m_offsets[y * m_tilesX + x + 1];
      }
    }
  }

  for (std::size_t i = 1; i < m_offsets.size(); ++i) {
    m_offsets[i] += m_offsets[i - 1];
  }

  m_indices.resize(m_offsets.back());
  m_cursor.assign(m_offsets.begin(), m_offsets.end() - 1);

  for (int i = 0; i < count; ++i) {
    const auto &L = lights[i];
    const float R = EffectiveRadius(L.attenuation);
    if (R <= 0.0f)
      continue;

    const float px = L.position.x;
    const float py = L.position.y;

    const int x0 = std::max(
        0, static_cast<int>(std::floor((px - R - m_origin.x) / m_tileW)));
    const int x1 = std::min(
        static_cast<int>(m_tilesX) - 1,
        static_cast<int>(std::floor((px + R - m_origin.x) / m_tileW)));

    const int y0 = std::max(
        0, static_cast<int>(std::floor((py - R - m_origin.y) / m_tileH)));
    const int y1 = std::min(
        static_cast<int>(m_tilesY) - 1,
        static_cast<int>(std::floor((py + R - m_origin.y) / m_tileH)));

    for (int y = y0; y <= y1; ++y) {
      for (int x = x0; x <= x1; ++x) {
        const std::uint32_t t = y * m_tilesX + x;
        m_indices[m_cursor[t]++] = static_cast<std::uint32_t>(i);
      }
    }
  }
}

} // namespace engine::graphics

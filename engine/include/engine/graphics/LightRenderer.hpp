#pragma once

#include "engine/graphics/LightBuffer.hpp"
#include <cstdint>

namespace engine::graphics {

class LightRenderer {
public:
  static constexpr std::uint32_t kPointBinding = 0;
  static constexpr std::uint32_t kDirectionalBinding = 1;
  static constexpr std::uint32_t kSpotBinding = 2;
  static constexpr std::uint32_t kTileOffsetBinding = 3;
  static constexpr std::uint32_t kTileIndexBinding = 4;

  LightRenderer();
  ~LightRenderer();
  LightRenderer(const LightRenderer &) = delete;
  LightRenderer &operator=(const LightRenderer &) = delete;

  void Upload(const LightBuffer &buf);

  void UploadTileGrid(const std::uint32_t *offsets, std::size_t offsetCount,
                      const std::uint32_t *indices, std::size_t indexCount);

private:
  std::uint32_t m_pointUbo = 0;
  std::uint32_t m_directionalUbo = 0;
  std::uint32_t m_spotUbo = 0;
  std::uint32_t m_tileOffsetSsbo = 0;
  std::uint32_t m_tileIndexSsbo = 0;
  std::size_t m_tileOffsetCap = 0;
  std::size_t m_tileIndexCap = 0;
};

} // namespace engine::graphics

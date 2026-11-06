#pragma once

#include "engine/graphics/assets/TextureAsset.hpp"
#include <cstdint>
#include <string>

namespace engine::graphics {

class Texture {
public:
  Texture() = default;
  ~Texture();

  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;
  Texture(Texture &&) noexcept;
  Texture &operator=(Texture &&) noexcept;

  bool LoadFromFile(const std::string &path);

  // Solid RGBA color
  void CreateSolid(std::uint32_t rgba);

  void CreateCheckerboard(int cells, std::uint32_t colorA,
                          std::uint32_t colorB);

  void Upload(int width, int height, ImageFormat format,
              const std::uint8_t *pixels, bool generateMipmaps = true);

  void Bind(std::uint32_t unit = 0) const;

  bool IsValid() const { return m_Id != 0; }
  std::uint32_t Id() const { return m_Id; }
  int Width() const { return m_Width; }
  int Height() const { return m_Height; }

private:
  void Release();

  std::uint32_t m_Id = 0;
  int m_Width = 0;
  int m_Height = 0;
};

} // namespace engine::graphics

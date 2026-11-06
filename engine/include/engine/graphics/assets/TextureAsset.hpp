#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/utils/Buffer.hpp"

namespace engine::graphics {

enum class ImageFormat {
  None = 0,
  R8,
  RGB8,
  RGBA8,
  RGBA32F,
};

struct TextureSpecification {
  std::uint32_t width = 1;
  std::uint32_t height = 1;
  ImageFormat format = ImageFormat::RGBA8;
  bool magFilterLinear = false;
};

class Texture : public asset::Asset {
public:
  virtual ~Texture() = default;

  virtual const TextureSpecification &GetSpecification() const = 0;

  virtual uint32_t GetWidth() const = 0;
  virtual uint32_t GetHeight() const = 0;
  virtual uint32_t GetRendererID() const = 0;

  virtual void SetData(Buffer data) = 0;

  virtual void Bind(uint32_t slot = 0) const = 0;

  virtual bool operator==(const Texture &other) const = 0;
  virtual bool equals(const Texture &other) const = 0;
};

} // namespace engine::graphics

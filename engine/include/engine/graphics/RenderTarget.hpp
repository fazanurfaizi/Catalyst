#pragma once

#include <cstdint>
namespace engine::graphics {

class RenderTarget {
public:
  RenderTarget() = default;
  ~RenderTarget();

  RenderTarget(const RenderTarget &) = delete;
  RenderTarget &operator=(const RenderTarget &) = delete;

  void Create(int w, int h);

  void Resize(int w, int h);

  void Bind(int viewportX, int viewportY, int viewportW, int viewportH) const;
  void Unbind(int viewportX, int viewportY, int viewportW, int viewportH) const;

  void BindColorTexture(std::uint32_t unit) const;

  std::uint32_t Fbo() const { return m_fbo; }
  std::uint32_t ColorTexture() const { return m_color; }

  int Width() const { return m_width; }
  int Height() const { return m_height; }

private:
  void Release();

  std::uint32_t m_fbo = 0;
  std::uint32_t m_color = 0;
  std::uint32_t m_depth = 0;
  int m_width = 0, m_height = 0;
};

} // namespace engine::graphics

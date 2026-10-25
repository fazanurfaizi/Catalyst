#pragma once

#include "engine/graphics/Camera.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>

namespace engine {

class SpriteBatch;
class Scene;
struct Transform;
struct Sprite;

class Renderer {
public:
  explicit Renderer(std::size_t maxQuads = 4096);
  ~Renderer();

  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

  void Begin(const Camera &camera);
  void End();

  void Draw(const Transform &t, const Sprite &s);
  void DrawScene(const Scene &scene);
  // void DrawText(const Font &font, const std::string &text,
  //               const glm::vec2 &position,
  //               const glm::vec4 &color = {1, 1, 1, 1}, float scale = 1.0f);

  void SetCullingEnabled(bool enabled) { m_cullEnabled = enabled; }
  bool CullingEnabled() const { return m_cullEnabled; }

  std::uint32_t DrawCalls() const;
  std::uint32_t QuadCount() const;
  std::uint32_t CulledCount() const;

private:
  std::unique_ptr<SpriteBatch> m_batch;
  const Camera *m_camera = nullptr;
  std::uint32_t m_lastCulled = 0;
  bool m_cullEnabled = true;
};

} // namespace engine

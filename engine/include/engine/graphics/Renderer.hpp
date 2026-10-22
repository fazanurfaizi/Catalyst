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

  std::uint32_t DrawCalls() const;
  std::uint32_t QuadCount() const;

private:
  std::unique_ptr<SpriteBatch> m_batch;
};

} // namespace engine

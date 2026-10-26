#pragma once

namespace engine::core {

class Game;

class GameState {
public:
  virtual ~GameState() = default;

  // Lifecycle
  virtual void OnEnter(Game &game) {}
  virtual void OnExit(Game &game) {}

  // Frame loop
  virtual void OnUpdate(Game &game, double deltaTime) = 0;
  virtual void OnRender(Game &game) = 0;
  virtual void OnImGui(Game &game) {}

  // Input forwarding
  virtual void OnKey(Game &game, int key, int scancode, int action, int mods) {}
  virtual void OnMouseButton(Game &game, int button, int action, int mods) {}
  virtual void OnMouseMove(Game &game, double x, double y) {}
  virtual void OnScroll(Game &game, double dx, double dy) {}
  virtual void OnResize(Game &game, int width, int height) {}
};

} // namespace engine::core

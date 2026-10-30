#pragma once

#include <memory>

#include "engine/core/GameState.hpp"

namespace engine::core {

class Game;

class GameStateManager {
public:
  GameStateManager();
  ~GameStateManager();

  void ChangeState(Game &game, std::unique_ptr<GameState> nextState);

  void Update(Game &game, double deltaTime);
  void FixedUpdate(Game &game, double fixedDeltaTime);
  void LateUpdate(Game &game, double deltaTime);

  void Render(Game &game);
  void Clear(Game &game);

  // Input Forwarding
  virtual void OnKey(Game &game, input::Key key, int scancode,
                     input::KeyAction action, int mods);
  virtual void OnMouseButton(Game &game, input::MouseButton button,
                             input::KeyAction action, int mods);
  void OnMouseMove(Game &game, double x, double y);
  void OnScroll(Game &game, double dx, double dy);
  void OnResize(Game &game, int width, int height);
  void OnImGui(Game &game);

  GameState *CurrentState() const { return m_current.get(); }

private:
  std::unique_ptr<GameState> m_current = nullptr;
  std::unique_ptr<GameState> m_pending = nullptr;
  bool m_hasPending = false;
};

} // namespace engine::core

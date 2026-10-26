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
  void Render(Game &game);

  // Input Forwarding
  void OnKey(Game &game, int key, int scancode, int action, int mods);
  void OnMouseButton(Game &game, int button, int action, int mods);
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

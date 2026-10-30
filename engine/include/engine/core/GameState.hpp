#pragma once

#include "engine/input/InputCodes.hpp"
namespace engine::core {

class Game;

class GameState {
public:
  virtual ~GameState() = default;

  // Lifecycle
  virtual void OnEnter(Game &game) {}
  virtual void OnExit(Game &game) {}

  /**
   * @brief Once per frame (matches game FPS)
   * Reading player input, non-physics movement, simple timers
   * example Input.GetKeyDown, transform.Translate
   */
  virtual void OnUpdate(Game &game, double deltaTime) = 0;

  /**
   * @brief Fixed interval (default: 0.02s / 50Hz)
   * Physics simulations, Rigidbody interactions
   * Rigidbody.AddForce, modifying physical velocity
   */
  virtual void OnFixedUpdate(Game &game, double fixedDeltaTime) {};

  /**
   * @brief Once per frame (after all Update calls finish)
   * Camera tracking, UI positioning, following targets
   * Third-person camera following a moving character
   */
  virtual void OnLateUpdate(Game &game, double deltaTime) {};

  virtual void OnRender(Game &game) = 0;
  virtual void OnImGui(Game &game) {}

  // Input forwarding
  virtual void OnKey(Game &game, input::Key key, int scancode,
                     input::KeyAction action, int mods) {}
  virtual void OnMouseButton(Game &game, input::MouseButton button,
                             input::KeyAction, int mods) {}
  virtual void OnMouseMove(Game &game, double x, double y) {}
  virtual void OnScroll(Game &game, double dx, double dy) {}
  virtual void OnResize(Game &game, int width, int height) {}
};

} // namespace engine::core

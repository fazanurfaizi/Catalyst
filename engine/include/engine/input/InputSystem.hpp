#pragma once

#include "InputState.hpp"

namespace engine::input {

class InputSystem {
public:
  void BeginFrame();
  void EndFrame();

  void OnGlfwKey(int glfwKey, int glfwAction);
  void OnGlfwMouseButton(int glfwButton, int glfwAction);
  void OnGlfwCursor(double x, double y);
  void OnGlfwScroll(double x, double y);

  const InputState &State() const { return m_state; }
  InputState &State() { return m_state; }

  bool IsKeyDown(Key k) const { return m_state.IsKeyDown(k); }
  bool IsMouseDown(MouseButton b) const { return m_state.IsMouseDown(b); }

  glm::vec2 CursorPos() const { return m_state.cursorPos; }
  glm::vec2 CursorDelta() const { return m_state.cursorDelta; }
  glm::vec2 ScrollDelta() const { return m_state.scrollDelta; }

  void RequestedClose() { m_state.closeRequested = true; }
  bool CloseRequested() const { return m_state.closeRequested; }

private:
  InputState m_state;
  bool m_firstCursorEvent = true;
};

} // namespace engine::input

#include "engine/input/InputSystem.hpp"
#include "engine/input/InputCodes.hpp"

#include <GLFW/glfw3.h>

namespace engine::input {

void InputSystem::BeginFrame() {
  m_state.cursorDelta = glm::vec2{0.0f};
  m_state.scrollDelta = glm::vec2{0.0f};
}

void InputSystem::OnGlfwKey(int glfwKey, int glfwAction) {
  const Key key = FromGlfwKey(glfwKey);
  if (key == Key::Unknown)
    return;

  const auto index = static_cast<std::size_t>(key);
  switch (glfwAction) {
  case GLFW_PRESS:
    m_state.keysDown[index] = true;
    break;
  case GLFW_RELEASE:
    m_state.keysDown[index] = false;
    break;
  case GLFW_REPEAT:
    break;
  }
}

void InputSystem::OnGlfwMouseButton(int glfwButton, int glfwAction) {
  const MouseButton button = FromGlfwMouseButton(glfwButton);
  if (button == MouseButton::Count)
    return;

  const auto index = static_cast<std::size_t>(button);
  if (glfwAction == GLFW_PRESS)
    m_state.mouseDown[index] = true;
  if (glfwAction == GLFW_RELEASE)
    m_state.mouseDown[index] = false;
}

void InputSystem::OnGlfwCursor(double x, double y) {
  const glm::vec2 next{static_cast<float>(x), static_cast<float>(y)};
  if (m_firstCursorEvent) {
    m_firstCursorEvent = false;
    m_state.cursorPos = next;
    return;
  }
  m_state.cursorDelta += next - m_state.cursorPos;
  m_state.cursorPos = next;
}

void InputSystem::OnGlfwScroll(double x, double y) {
  m_state.scrollDelta +=
      glm::vec2{static_cast<float>(x), static_cast<float>(y)};
}

KeyAction FromGlfwAction(int glfwAction) {
  switch (glfwAction) {
  case GLFW_PRESS:
    return KeyAction::Press;
  case GLFW_RELEASE:
    return KeyAction::Release;
  case GLFW_REPEAT:
    return KeyAction::Repeat;
  }
  return KeyAction::Release;
}

MouseButton FromGlfwMouseButton(int glfwButton) {
  switch (glfwButton) {
  case GLFW_MOUSE_BUTTON_LEFT:
    return MouseButton::Left;
  case GLFW_MOUSE_BUTTON_RIGHT:
    return MouseButton::Right;
  case GLFW_MOUSE_BUTTON_MIDDLE:
    return MouseButton::Middle;
  case GLFW_MOUSE_BUTTON_4:
    return MouseButton::Button4;
  case GLFW_MOUSE_BUTTON_5:
    return MouseButton::Button5;
  }
  return MouseButton::Count;
}

Key FromGlfwKey(int glfwKey) {
  switch (glfwKey) {
    // clang-format off
    case GLFW_KEY_A: return Key::A; case GLFW_KEY_B: return Key::B;
    case GLFW_KEY_C: return Key::C; case GLFW_KEY_D: return Key::D;
    case GLFW_KEY_E: return Key::E; case GLFW_KEY_F: return Key::F;
    case GLFW_KEY_G: return Key::G; case GLFW_KEY_H: return Key::H;
    case GLFW_KEY_I: return Key::I; case GLFW_KEY_J: return Key::J;
    case GLFW_KEY_K: return Key::K; case GLFW_KEY_L: return Key::L;
    case GLFW_KEY_M: return Key::M; case GLFW_KEY_N: return Key::N;
    case GLFW_KEY_O: return Key::O; case GLFW_KEY_P: return Key::P;
    case GLFW_KEY_Q: return Key::Q; case GLFW_KEY_R: return Key::R;
    case GLFW_KEY_S: return Key::S; case GLFW_KEY_T: return Key::T;
    case GLFW_KEY_U: return Key::U; case GLFW_KEY_V: return Key::V;
    case GLFW_KEY_W: return Key::W; case GLFW_KEY_X: return Key::X;
    case GLFW_KEY_Y: return Key::Y; case GLFW_KEY_Z: return Key::Z;

    case GLFW_KEY_0: return Key::Digit0; case GLFW_KEY_1: return Key::Digit1;
    case GLFW_KEY_2: return Key::Digit2; case GLFW_KEY_3: return Key::Digit3;
    case GLFW_KEY_4: return Key::Digit4; case GLFW_KEY_5: return Key::Digit5;
    case GLFW_KEY_6: return Key::Digit6; case GLFW_KEY_7: return Key::Digit7;
    case GLFW_KEY_8: return Key::Digit8; case GLFW_KEY_9: return Key::Digit9;

    case GLFW_KEY_LEFT:  return Key::Left;  case GLFW_KEY_RIGHT: return Key::Right;
    case GLFW_KEY_UP:    return Key::Up;    case GLFW_KEY_DOWN:  return Key::Down;

    case GLFW_KEY_ESCAPE:    return Key::Escape;
    case GLFW_KEY_ENTER:     return Key::Enter;
    case GLFW_KEY_SPACE:     return Key::Space;
    case GLFW_KEY_TAB:       return Key::Tab;
    case GLFW_KEY_BACKSPACE: return Key::Backspace;
    case GLFW_KEY_DELETE:    return Key::Delete;

    case GLFW_KEY_LEFT_SHIFT:  return Key::LeftShift;
    case GLFW_KEY_RIGHT_SHIFT: return Key::RightShift;
    case GLFW_KEY_LEFT_CONTROL:  return Key::LeftCtrl;
    case GLFW_KEY_RIGHT_CONTROL: return Key::RightCtrl;
    case GLFW_KEY_LEFT_ALT:  return Key::LeftAlt;
    case GLFW_KEY_RIGHT_ALT: return Key::RightAlt;

    case GLFW_KEY_F1:  return Key::F1;  case GLFW_KEY_F2:  return Key::F2;
    case GLFW_KEY_F3:  return Key::F3;  case GLFW_KEY_F4:  return Key::F4;
    case GLFW_KEY_F5:  return Key::F5;  case GLFW_KEY_F6:  return Key::F6;
    case GLFW_KEY_F7:  return Key::F7;  case GLFW_KEY_F8:  return Key::F8;
    case GLFW_KEY_F9:  return Key::F9;  case GLFW_KEY_F10: return Key::F10;
    case GLFW_KEY_F11: return Key::F11; case GLFW_KEY_F12: return Key::F12;

    case GLFW_KEY_GRAVE_ACCENT: return Key::Grave;
    case GLFW_KEY_MINUS:        return Key::Minus;
    case GLFW_KEY_EQUAL:        return Key::Equal;
    case GLFW_KEY_LEFT_BRACKET: return Key::LeftBracket;
    case GLFW_KEY_RIGHT_BRACKET:return Key::RightBracket;
    case GLFW_KEY_BACKSLASH:    return Key::Backslash;
    case GLFW_KEY_SEMICOLON:    return Key::Semicolon;
    case GLFW_KEY_APOSTROPHE:   return Key::Apostrophe;
    case GLFW_KEY_COMMA:        return Key::Comma;
    case GLFW_KEY_PERIOD:       return Key::Period;
    case GLFW_KEY_SLASH:        return Key::Slash;
    // clang-format on
  }
  return Key::Unknown;
}

} // namespace engine::input

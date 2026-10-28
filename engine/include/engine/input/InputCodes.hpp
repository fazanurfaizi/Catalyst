#pragma once

namespace engine::input {

// clang-format off
enum class Key : int {
  Unknown = 0,
  // Letters
  A, B, C, D, E, F, G, H, I, J, K, L, M,
  N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
  // Digits
  Digit0, Digit1, Digit2, Digit3, Digit4,
  Digit5, Digit6, Digit7, Digit8, Digit9,
  // Arrows
  Left, Right, Up, Down,
  // Control
  Escape, Enter, Space, Tab, Backspace, Delete,
  // Modifiers
  LeftShift, RightShift, LeftCtrl, RightCtrl, LeftAlt, RightAlt,
  // Function
  F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
  // Punctuation
  Grave, Minus, Equal,
  LeftBracket, RightBracket, Backslash,
  Semicolon, Apostrophe, Comma, Period, Slash,

  Count
};
// clang-format on

enum class MouseButton : int {
  Left = 0,
  Right,
  Middle,
  Button4,
  Button5,
  Count
};

enum class KeyAction { Release, Press, Repeat };

Key FromGlfwKey(int glfwKey);
MouseButton FromGlfwMouseButton(int glfwButton);
KeyAction FromGlfwAction(int glfwAction);

} // namespace engine::input

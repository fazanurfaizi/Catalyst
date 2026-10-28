#pragma once

#include "InputCodes.hpp"

#include <array>
#include <cstddef>

#include <glm/glm.hpp>

namespace engine::input {

struct InputState {
  std::array<bool, static_cast<std::size_t>(Key::Count)> keysDown{};
  std::array<bool, static_cast<std::size_t>(MouseButton::Count)> mouseDown{};

  glm::vec2 cursorPos{};
  glm::vec2 cursorDelta{};
  glm::vec2 scrollDelta{};

  bool closeRequested = false;

  bool IsKeyDown(Key k) const { return keysDown[static_cast<std::size_t>(k)]; }
  bool IsMouseDown(MouseButton b) const {
    return mouseDown[static_cast<std::size_t>(b)];
  }
};

} // namespace engine::input

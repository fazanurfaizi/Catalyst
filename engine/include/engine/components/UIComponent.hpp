#pragma once

#include "engine/components/RenderLayer.hpp"
#include "engine/math/Color.hpp"
#include <glm/glm.hpp>

namespace engine::components {

struct UIRect {
  glm::vec2 position{0.0f, 0.0f};
  glm::vec2 size{100.0f, 50.0f};
};

struct UIPanel {
  math::Color color{1.0f, 1.0f, 1.0f, 1.0f};
  RenderLayer layer = RenderLayer::Background;
};

struct UIButton {
  math::Color normalColor{0.2f, 0.2f, 0.2f, 1.0f};
  math::Color hoverColor{0.3f, 0.3f, 0.3f, 1.0f};
  math::Color pressedColor{0.1f, 0.1f, 0.1f, 1.0f};

  bool isHovered = false;
  bool isPressed = false;
  bool wasClicked = false;
};

} // namespace engine::components

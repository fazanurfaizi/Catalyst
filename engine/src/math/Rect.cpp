#include "engine/math/Rect.hpp"

namespace engine::math {

bool Rect::Contains(const glm::vec2 &p) const {
  const bool hitX = p.x >= x && p.x <= x + width;
  const bool hitY = p.y >= y && p.y <= y + height;
  const bool hit = hitX && hitY;

  return p.x >= this->x && p.x <= this->x + this->width && p.y >= this->y &&
         p.y <= this->y + height;
}

bool Rect::Intersects(const Rect &other) const {
  return !(other.x > this->Right() || other.Right() < this->x ||
           other.y > this->Bottom() || other.Bottom() < this->y);
}

} // namespace engine::math

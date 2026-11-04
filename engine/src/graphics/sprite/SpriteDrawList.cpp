#include "engine/graphics/sprite/SpriteDrawList.hpp"

#include <cstddef>

namespace engine::graphics {

void SpriteDrawList::Clear() {
  m_commands.clear();
  m_groups.clear();
}

void SpriteDrawList::Push(const glm::vec2 &pos, const glm::vec2 &scale,
                          float rotation, const math::Color &color,
                          const Texture *texture, const glm::vec2 &uvMin,
                          const glm::vec2 &uvMax) {
  SpriteCommand c{};
  c.instance.position = pos;
  c.instance.scale = scale;
  c.instance.rotation = rotation;
  c.instance.color = color.toVec4();
  c.instance.uvRect = {uvMin.x, uvMin.y, uvMax.x, uvMax.y};
  c.texture = texture;
  m_commands.push_back(c);
}

void SpriteDrawList::BuildBatches() {
  m_groups.clear();
  if (m_commands.empty())
    return;

  std::size_t start = 0;
  for (std::size_t i = 1; i <= m_commands.size(); ++i) {
    if (i == m_commands.size() ||
        m_commands[i].texture != m_commands[start].texture) {

      m_groups.push_back({
          start,
          i - start,
          m_commands[start].texture,
      });
      start = i;
    }
  }
}

} // namespace engine::graphics

#pragma once

#include "engine/graphics/texture/Texture.hpp"
#include "engine/math/Color.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace engine::graphics {

struct SpriteInstance {
  glm::vec2 position;
  glm::vec2 scale;
  float rotation;
  float _pad[3];
  glm::vec4 color;
  glm::vec4 uvRect;
};

static_assert(sizeof(SpriteInstance) == 64, "SpriteInstance must be 64");

struct SpriteCommand {
  SpriteInstance instance;
  const Texture *texture;
};

class SpriteDrawList {
public:
  struct Group {
    std::size_t start;
    std::size_t count;
    const Texture *texture;
  };

  void Clear();
  void Push(const glm::vec2 &pos, const glm::vec2 &scale, float rotation,
            const math::Color &color, const Texture *texture,
            const glm::vec2 &uvMin, const glm::vec2 &uvMax);
  void BuildBatches();

  std::size_t Size() const { return m_commands.size(); }
  const std::vector<Group> &Groups() const { return m_groups; }
  const std::vector<SpriteCommand> &Commands() const { return m_commands; }

private:
  std::vector<SpriteCommand> m_commands;
  std::vector<Group> m_groups;
};

} // namespace engine::graphics

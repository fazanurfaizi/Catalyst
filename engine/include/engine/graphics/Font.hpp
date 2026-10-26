#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <map>
#include <string>

namespace engine::graphics {

struct Character {
  GLuint textureId;
  glm::ivec2 size;
  glm::ivec2 bearing;
  float advance;
  glm::vec2 uvMin;
  glm::vec2 uvMax;
};

class Font {
public:
  bool LoadFromFile(const std::string &path, float fontSize);

  const Character &GetCharacter(char c) const;
  GLuint GetTextureId() const { return m_textureId; }

private:
  GLuint m_textureId = 0;
  std::map<char, Character> m_characters;
};

} // namespace engine::graphics

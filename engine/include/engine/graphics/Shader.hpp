#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <string_view>
#include <unordered_map>

namespace engine::graphics {

class Shader {
public:
  Shader() = default;
  ~Shader();

  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;
  Shader(Shader &&) noexcept;
  Shader &operator=(Shader &&) noexcept;

  bool LoadFromFile(const std::string &path);
  bool Compile(const std::string &vertexSrc, const std::string &fragmentSrc);

  void Bind() const;
  void Unbind() const;

  bool IsValid() const { return m_Program != 0; }
  std::uint32_t ProgramId() const { return m_Program; }
  int Location(const char *name) const;

  void SetMat4(const char *name, const glm::mat4 &value);
  void SetMat3(const char *name, const glm::mat3 &value);
  void SetVec4(const char *name, const glm::vec4 &value);
  void SetVec3(const char *name, const glm::vec3 &value);
  void SetVec2(const char *name, const glm::vec2 &value);
  void SetFloat(const char *name, float value);
  void SetInt(const char *name, int value);
  void SetUInt(const char *name, unsigned int value);
  void SetUVec2(const char *name, const glm::uvec2 &value);

private:
  std::uint32_t m_Program = 0;
  mutable std::unordered_map<std::string_view, int> m_uniformCache;
};

} // namespace engine::graphics

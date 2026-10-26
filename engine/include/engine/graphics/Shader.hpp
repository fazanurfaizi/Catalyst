#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <string>

namespace engine::graphics {

class Shader {
public:
  Shader() = default;
  ~Shader();

  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;
  Shader(Shader &&) noexcept;
  Shader &operator=(Shader &&) noexcept;

  bool Compile(const std::string &vertexSrc, const std::string &fragmentSrc);

  void Bind() const;
  void Unbind() const;

  bool IsValid() const { return m_Program != 0; }
  std::uint32_t ProgramId() const { return m_Program; }

  void SetMat4(const char *name, const glm::mat4 &value);
  void SetVec4(const char *name, const glm::vec4 &value);
  void SetVec2(const char *name, const glm::vec2 &value);
  void SetFloat(const char *name, float value);
  void SetInt(const char *name, int value);

private:
  std::uint32_t m_Program = 0;
};

} // namespace engine::graphics

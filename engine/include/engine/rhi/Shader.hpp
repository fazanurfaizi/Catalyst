#pragma once

#include <glm/glm.hpp>

#include <memory>

namespace engine::graphics {

class Shader {
public:
  Shader() = default;
  virtual ~Shader() = default;

  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;
  Shader(Shader &&) noexcept;
  Shader &operator=(Shader &&) noexcept;

  void Bind() const;
  void Unbind() const;

  void SetFloat(const char *name, float value);
  void SetVec2(const char *name, const glm::vec2 &value);
  void SetVec3(const char *name, const glm::vec3 &value);
  void SetVec4(const char *name, const glm::vec4 &value);
  void SetInt(const char *name, int value);
  void SetIVec2(const char *name, const glm::ivec2 &value);
  void SetIVec3(const char *name, const glm::ivec3 &value);
  void SetIVec4(const char *name, const glm::ivec4 &value);
  void SetMat2(const char *name, const glm::mat2 &value);
  void SetMat3(const char *name, const glm::mat3 &value);
  void SetMat4(const char *name, const glm::mat4 &value);
  void SetUInt(const char *name, unsigned int value);

  virtual const std::string &Name() const = 0;

  static std::shared_ptr<Shader> Create(const std::string &filepath);
  static std::shared_ptr<Shader> Create(const std::string &name,
                                        const std::string &vertexSrc,
                                        const std::string &fragmentSrc);
};

} // namespace engine::graphics

#include "engine/graphics/Shader.hpp"

#include "engine/core/Logger.hpp"

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

namespace engine {

namespace {
GLuint CompileStage(GLenum type, const std::string &src) {
  const GLuint shader = glCreateShader(type);
  if (shader == 0)
    return 0;

  const char *srcPtr = src.c_str();
  glShaderSource(shader, 1, &srcPtr, nullptr);
  glCompileShader(shader);

  GLint ok = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    GLint len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    std::vector<char> log(static_cast<std::size_t>(len > 0 ? len : 1));
    glGetShaderInfoLog(shader, len, nullptr, log.data());
    Logger::Error("[Shader] %s compile failed:\n %s",
                  type == GL_VERTEX_SHADER ? "vertex" : "fragment", log.data());
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}
} // namespace

Shader::~Shader() {
  if (m_Program)
    glDeleteProgram(m_Program);
}

Shader::Shader(Shader &&other) noexcept : m_Program(other.m_Program) {
  other.m_Program = 0;
}

Shader &Shader::operator=(Shader &&other) noexcept {
  if (this != &other) {
    if (m_Program)
      glDeleteProgram(m_Program);
    m_Program = other.m_Program;
    other.m_Program = 0;
  }
  return *this;
}

bool Shader::Compile(const std::string &vertexSrc,
                     const std::string &fragmentSrc) {
  const GLuint vs = CompileStage(GL_VERTEX_SHADER, vertexSrc);
  const GLuint fs = CompileStage(GL_FRAGMENT_SHADER, fragmentSrc);

  const GLuint program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  glDeleteShader(vs);
  glDeleteShader(fs);

  GLint ok = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &ok);
  if (!ok) {
    GLint len = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
    std::vector<char> log(static_cast<std::size_t>(len > 0 ? len : 1));
    glGetProgramInfoLog(program, len, nullptr, log.data());
    Logger::Error("[Shader] link failed:\n %s", log.data());
    glDeleteProgram(program);
    return false;
  }

  if (m_Program)
    glDeleteProgram(m_Program);
  m_Program = program;
  return true;
}

void Shader::Bind() const { glUseProgram(m_Program); }
void Shader::Unbind() const { glUseProgram(0); }

void Shader::SetMat4(const char *name, const glm::mat4 &v) {
  glUniformMatrix4fv(glGetUniformLocation(m_Program, name), 1, GL_FALSE,
                     glm::value_ptr(v));
}
void Shader::SetVec4(const char *name, const glm::vec4 &v) {
  glUniform4fv(glGetUniformLocation(m_Program, name), 1, glm::value_ptr(v));
}
void Shader::SetVec2(const char *name, const glm::vec2 &v) {
  glUniform2fv(glGetUniformLocation(m_Program, name), 1, glm::value_ptr(v));
}
void Shader::SetFloat(const char *name, float v) {
  glUniform1f(glGetUniformLocation(m_Program, name), v);
}
void Shader::SetInt(const char *name, int v) {
  glUniform1i(glGetUniformLocation(m_Program, name), v);
}

} // namespace engine

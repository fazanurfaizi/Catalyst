#include "engine/graphics/gl/Shader.hpp"

#include "engine/utils/Logger.hpp"

#include <filesystem>
#include <fstream>
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include <iosfwd>
#include <vector>

namespace engine::graphics {

namespace {
GLuint CompileStage(GLenum type, const std::string &src) {
  const GLuint shader = glCreateShader(type);
  if (shader == 0) {
    Logger::Error("[Shader] Failed to create shader");
    return 0;
  }

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

bool ReadFile(const std::filesystem::path &path, std::string &out) {
  std::ifstream in(path, std::ios::binary);
  if (!in)
    return false;

  in.seekg(0, std::ios::end);

  const std::streampos size = in.tellg();
  if (size < 0)
    return false;

  out.resize(static_cast<std::size_t>(size));
  in.seekg(0, std::ios::beg);
  in.read(out.data(), size);
  return in.good() || in.eof();
}
} // namespace

Shader::~Shader() {
  if (m_Program)
    glDeleteProgram(m_Program);
}

Shader::Shader(Shader &&other) noexcept
    : m_Program(other.m_Program),
      m_uniformCache(std::move(other.m_uniformCache)) {
  other.m_Program = 0;
}

Shader &Shader::operator=(Shader &&other) noexcept {
  if (this != &other) {
    if (m_Program)
      glDeleteProgram(m_Program);
    m_Program = other.m_Program;
    m_uniformCache = std::move(other.m_uniformCache);
    other.m_Program = 0;
  }
  return *this;
}

bool Shader::LoadFromFile(const std::string &path) {
  namespace fs = std::filesystem;

  fs::path base(path);
  while (base.has_extension()) {
    base.replace_extension();
  }

  fs::path vertPath, fragPath;
  std::error_code ec;
  if (fs::is_directory(base, ec) && !ec) {
    // example directory-per shader:
    // assets/shaders/light/light.vert.glsl
    // assets/shaders/light/light.frag.glsl
    const std::string stem = base.filename().string();
    vertPath = base / (stem + ".vert.glsl");
    fragPath = base / (stem + ".frag.glsl");
  } else {
    // example: assets/shaders/sprite.vert.glsl
    vertPath = base.string() + ".vert.glsl";
    fragPath = base.string() + ".frag.glsl";
  }

  std::string vertSrc, fragSrc;
  if (!ReadFile(vertPath, vertSrc)) {
    Logger::Error("[Shader] cannot read vertex shader: %s", vertPath.c_str());
    return false;
  }
  if (!ReadFile(fragPath, fragSrc)) {
    Logger::Error("[Shader] cannot read fragment shader: %s", fragPath.c_str());
    return false;
  }

  return Compile(vertSrc, fragSrc);
}

bool Shader::Compile(const std::string &vertexSrc,
                     const std::string &fragmentSrc) {
  const GLuint vs = CompileStage(GL_VERTEX_SHADER, vertexSrc);
  if (vs == 0)
    return false;

  const GLuint fs = CompileStage(GL_FRAGMENT_SHADER, fragmentSrc);
  if (fs == 0) {
    glDeleteShader(vs);
    return false;
  };

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
  m_uniformCache.clear();
  return true;
}

void Shader::Bind() const { glUseProgram(m_Program); }
void Shader::Unbind() const { glUseProgram(0); }

int Shader::Location(const char *name) const {
  if (m_Program == 0)
    return -1;

  auto it = m_uniformCache.find(name);

  if (it != m_uniformCache.end())
    return it->second;
  const int loc = glGetUniformLocation(m_Program, name);
#ifndef NDEBUG
  if (loc < 0)
    Logger::Warn("[Shader] uniform '%s' not found in program %u", name,
                 m_Program);
#endif
  m_uniformCache.emplace(name, loc);
  return loc;
}

void Shader::SetMat4(const char *name, const glm::mat4 &v) {
  glUniformMatrix4fv(Location(name), 1, GL_FALSE, glm::value_ptr(v));
}
void Shader::SetMat3(const char *name, const glm::mat3 &v) {
  glUniformMatrix3fv(Location(name), 1, GL_FALSE, glm::value_ptr(v));
}
void Shader::SetVec4(const char *name, const glm::vec4 &v) {
  glUniform4fv(Location(name), 1, glm::value_ptr(v));
}
void Shader::SetVec3(const char *name, const glm::vec3 &v) {
  glUniform3fv(Location(name), 1, glm::value_ptr(v));
}
void Shader::SetVec2(const char *name, const glm::vec2 &v) {
  glUniform2fv(Location(name), 1, glm::value_ptr(v));
}
void Shader::SetFloat(const char *name, float v) {
  glUniform1f(Location(name), v);
}
void Shader::SetInt(const char *name, int v) { glUniform1i(Location(name), v); }
void Shader::SetUInt(const char *name, unsigned int v) {
  glUniform1ui(Location(name), v);
}
void Shader::SetUVec2(const char *name, const glm::uvec2 &v) {
  glUniform2uiv(Location(name), 1, glm::value_ptr(v));
}

} // namespace engine::graphics

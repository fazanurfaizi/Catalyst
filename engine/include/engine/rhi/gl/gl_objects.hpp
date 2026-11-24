#pragma once

#include "glad/gl.h"
#include <utility>

namespace engine::rhi {

class DeviceImpl;

// RAII owner for a single OpenGL buffer name. Header-only on purpose: it has no
// dependencies beyond the name and is used by several backend files.
class GLBuffer final {
public:
  GLBuffer() = default;

  explicit GLBuffer(GLuint name, DeviceImpl *deviceImpl = nullptr) noexcept
      : m_deviceImpl{deviceImpl}, m_glName{name} {}

  ~GLBuffer() noexcept { Reset(); }

  GLBuffer(const GLBuffer &) = delete;
  GLBuffer &operator=(const GLBuffer &) = delete;

  GLBuffer(GLBuffer &&other) noexcept : m_deviceImpl{other.m_deviceImpl}, m_glName{other.m_glName} {
    other.m_glName = 0;
  }

  GLBuffer &operator=(GLBuffer &&other) noexcept {
    if (this != &other) {
      Reset();
      m_deviceImpl = other.m_deviceImpl;
      m_glName = other.m_glName;
      other.m_glName = 0;
    }
    return *this;
  }

  [[nodiscard]] GLuint GlName() const noexcept { return m_glName; }

private:
  void Reset() noexcept {
    if (m_glName != 0) {
      glDeleteBuffers(1, &m_glName);
      m_glName = 0;
    }
  }

  DeviceImpl *m_deviceImpl{nullptr};
  GLuint m_glName{0};
};

class GLTexture final {
public:
  explicit GLTexture(GLuint name, bool owned = true, DeviceImpl *deviceImpl = nullptr) noexcept
      : m_deviceImpl(deviceImpl), m_glName(name), m_owned(owned) {}

  ~GLTexture() noexcept { Reset(); }

  GLTexture(const GLTexture &) = delete;
  void operator=(const GLTexture &) = delete;

  GLTexture(GLTexture &&other) noexcept
      : m_deviceImpl(std::exchange(other.m_deviceImpl, nullptr)),
        m_glName(std::exchange(other.m_glName, 0)), m_owned(std::exchange(other.m_owned, false)) {}

  GLTexture &operator=(GLTexture &&other) noexcept {
    if (this != &other) {
      Reset();
      m_deviceImpl = other.m_deviceImpl;
      m_glName = other.m_glName;
      m_owned = other.m_owned;
      other.m_glName = 0;
      other.m_owned = false;
    }
    return *this;
  }

  [[nodiscard]] GLuint GlName() const noexcept { return m_glName; }

private:
  void Reset() {
    if (m_owned && (m_glName != 0)) {
      glDeleteTextures(1, &m_glName);
    }
  }

  DeviceImpl *m_deviceImpl{nullptr};
  GLuint m_glName{0};
  bool m_owned{true};
};

} // namespace engine::rhi

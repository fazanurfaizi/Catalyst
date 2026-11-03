#include "engine/platform/Window.hpp"
#include "engine/input/InputCodes.hpp"
#include "engine/utils/Logger.hpp"
#include "glad/gl.h"

#include <GLFW/glfw3.h>

#include <cstdio>

static_assert(engine::platform::kMaxKeys >
                  static_cast<std::size_t>(GLFW_KEY_LAST),
              "kMaxKeys too small - raise it in Window.hpp");

namespace {

void GlfwErrorCallback(int code, const char *desc) {
  std::fprintf(stderr, "GLFW error %d: %s\n", code, desc);
}

bool IsWayland() {
#if defined(GLFW_VERSION_MAJOR) && defined(GLFW_VERSION_MINOR) &&              \
    (GLFW_VERSION_MAJOR > 3 ||                                                 \
     (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 4))
  return glfwGetPlatform() == GLFW_PLATFORM_WAYLAND;
#else
  if (const char *t = std::getenv("XDG_SESSION_TYPE"))
    if (std::strcmp(t, "wayland") == 0)
      return true;
  return std::getenv("WAYLAND_DISPLAY") != nullptr;
#endif // defined (GLFW_VERSION_MAJOR) && defined (GLFW_VERSION_MINOR)
}

} // namespace

namespace engine::platform {

std::unique_ptr<Window> Window::Create(const WindowConfig &config) {
  auto self = std::unique_ptr<Window>(new Window(config));
  if (!self->m_window)
    return nullptr;
  return self;
}

Window::Window(const WindowConfig &config) : m_Config(config) {
  glfwSetErrorCallback(GlfwErrorCallback);
  if (!glfwInit()) {
    Logger::Error("glfwInit failed");
    return;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

  GLFWmonitor *monitor = glfwGetPrimaryMonitor();
  if (config.monitorIndex >= 0) {
    int count = 0;
    if (GLFWmonitor **ms = glfwGetMonitors(&count);
        count > 0 && config.monitorIndex < count) {
      monitor = ms[config.monitorIndex];
    }
  }

  m_savedW = m_Config.width;
  m_savedH = m_Config.height;

  if (config.fullscreen && monitor != nullptr) {
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    const int fw = mode ? mode->width : m_Config.width;
    const int fh = mode ? mode->height : m_Config.height;
    m_window =
        glfwCreateWindow(fw, fh, m_Config.title.c_str(), monitor, nullptr);
    m_fullscreen = true;
  } else {
    m_window = glfwCreateWindow(m_Config.width, m_Config.height,
                                m_Config.title.c_str(), nullptr, nullptr);
    m_fullscreen = false;
  }

  if (!m_window) {
    Logger::Error("glfwCreateWindow failed");
    glfwTerminate();
    return;
  }

  m_display.Initialize(m_window);

  glfwMakeContextCurrent(m_window);
  glfwSwapInterval(m_Config.vsync ? 1 : 0);

  glfwSetWindowUserPointer(m_window, this);
  glfwSetKeyCallback(m_window, &Window::ThunkKey);
  glfwSetMouseButtonCallback(m_window, &Window::ThunkMouseButton);
  glfwSetCursorPosCallback(m_window, &Window::ThunkCursorPos);
  glfwSetScrollCallback(m_window, &Window::ThunkScroll);
  glfwSetFramebufferSizeCallback(m_window, &Window::ThunkFramebufferResize);
  glfwSetWindowCloseCallback(m_window, &Window::ThunkClose);
}

Window::~Window() {
  m_display.Shutdown();
  if (m_window) {
    glfwDestroyWindow(m_window);
    m_window = nullptr;
  }
  glfwTerminate();
}

void Window::SetFullscreen(bool fullscreen) {
  if (!m_window || m_fullscreen == fullscreen)
    return;

  const bool wayland = IsWayland();

  if (fullscreen) {
    if (!wayland) {
      glfwGetWindowPos(m_window, &m_savedX, &m_savedY);
    } else {
      m_savedW = 0;
      m_savedH = 0;
    }
    glfwGetWindowSize(m_window, &m_savedW, &m_savedH);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    if (m_Config.monitorIndex >= 0) {
      int count = 0;
      if (GLFWmonitor **ms = glfwGetMonitors(&count);
          count > 0 && m_Config.monitorIndex < count) {
        monitor = ms[m_Config.monitorIndex];
      }
    }
    if (!monitor)
      return;

    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    if (!mode)
      return;

    glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height,
                         mode->refreshRate);
  } else {
    if (wayland) {
      glfwSetWindowMonitor(m_window, nullptr, GLFW_DONT_CARE, GLFW_DONT_CARE,
                           m_savedW, m_savedH, GLFW_DONT_CARE);
    }
    glfwSetWindowMonitor(m_window, nullptr, m_savedX, m_savedY, m_savedW,
                         m_savedH, GLFW_DONT_CARE);
  }

  m_fullscreen = fullscreen;
  m_Config.fullscreen = fullscreen;

  m_display.Refresh();
  m_Config.width = m_display.WindowSize().x;
  m_Config.height = m_display.WindowSize().y;
}

void Window::SetWindowedSize(int w, int h) {
  if (!m_window || m_fullscreen)
    return;
  if (w <= 0 || h <= 0)
    return;
  glfwSetWindowSize(m_window, w, h);
  m_savedW = w;
  m_savedH = h;
  m_Config.width = w;
  m_Config.height = h;
  m_display.Refresh();
}

void Window::SetVsync(bool enabled) {
  if (!m_window)
    return;
  glfwSwapInterval(enabled ? 1 : 0);
  m_Config.vsync = enabled;
}

bool Window::ShouldClose() const { return glfwWindowShouldClose(m_window); }

void Window::RequestClose() { glfwSetWindowShouldClose(m_window, GLFW_TRUE); }

void Window::PollEvents() { glfwPollEvents(); }

void Window::SwapBuffers() { glfwSwapBuffers(m_window); }

void Window::ClearWindow() {
  glClearColor(0.15f, 0.20f, 0.30f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void Window::SetTitle(std::string_view title) {
  glfwSetWindowTitle(m_window, std::string(title).c_str());
}

Window *Window::Self(GLFWwindow *w) {
  return static_cast<Window *>(glfwGetWindowUserPointer(w));
}

void Window::ThunkKey(GLFWwindow *w, int key, int sc, int action, int mods) {
  auto *s = Self(w);
  if (!s)
    return;

  s->m_input.OnGlfwKey(key, action);

  if (s->events.onKey)
    s->events.onKey(input::FromGlfwKey(key), sc, input::FromGlfwAction(action),
                    mods);
}

void Window::ThunkMouseButton(GLFWwindow *w, int b, int a, int m) {
  auto *s = Self(w);
  if (!s)
    return;

  s->m_input.OnGlfwMouseButton(b, a);

  if (s->events.onMouseButton) {
    s->events.onMouseButton(input::FromGlfwMouseButton(b),
                            input::FromGlfwAction(a), m);
  }
}

void Window::ThunkCursorPos(GLFWwindow *w, double x, double y) {
  auto *s = Self(w);
  if (!s)
    return;
  s->m_input.OnGlfwCursor(x, y);
  if (s->events.onCursorMove)
    s->events.onCursorMove(x, y);
}

void Window::ThunkScroll(GLFWwindow *w, double x, double y) {
  auto *s = Self(w);
  if (!s)
    return;
  s->m_input.OnGlfwScroll(x, y);
  if (s->events.onScroll)
    s->events.onScroll(x, y);
}

void Window::ThunkFramebufferResize(GLFWwindow *w, int width, int height) {
  auto *s = Self(w);
  if (!s)
    return;

  s->m_display.OnFramebufferResize(width, height);

  int ww, wh;
  glfwGetWindowSize(w, &ww, &wh);
  s->m_display.OnWindowResize(ww, wh);
  if (s->events.onResize)
    s->events.onResize(width, height);
}

void Window::ThunkClose(GLFWwindow *w) {
  if (auto *s = Self(w)) {
    s->m_input.RequestedClose();
    if (s->events.onClose)
      s->events.onClose();
  }
}

} // namespace engine::platform

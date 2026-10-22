#include "engine/core/Game.hpp"

#include "engine/core/Logger.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <cstdio>

namespace engine {

namespace {
void GlfwErrorCallback(int code, const char *desc) {
  std::fprintf(stderr, "GLFW error %d: %s\n", code, desc);
}
} // namespace

Game::Game(const WindowConfig &config) : m_Config(config) {
  glfwSetErrorCallback(GlfwErrorCallback);
  if (!glfwInit()) {
    Logger::Error("glfwInit failed");
    return;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  int monitorCount;
  GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

  if (monitorCount == 0) {
    Logger::Error("No monitors found");
    glfwTerminate();
    return;
  }

  int targetIndex = (monitorCount > 1) ? 1 : 0;
  GLFWmonitor *targetMonitor = monitors[targetIndex];

  const GLFWvidmode *mode = glfwGetVideoMode(targetMonitor);
  if (mode != nullptr) {
    m_Config.width = mode->width;
    m_Config.height = mode->height;
  }

  m_Window = glfwCreateWindow(m_Config.width, m_Config.height,
                              m_Config.title.c_str(), nullptr, nullptr);

  if (!m_Window) {
    Logger::Error("glfwCreateWindow failed");
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(m_Window);
  glfwSwapInterval(m_Config.vsync ? 1 : 0);

  glfwSetWindowUserPointer(m_Window, this);
  glfwSetKeyCallback(m_Window, &Game::ThunkKey);
  glfwSetMouseButtonCallback(m_Window, &Game::ThunkMouseButton);
  glfwSetCursorPosCallback(m_Window, &Game::ThunkCursorPos);
  glfwSetScrollCallback(m_Window, &Game::ThunkScroll);
  glfwSetFramebufferSizeCallback(m_Window, &Game::ThunkFramebufferSize);

  if (!gladLoadGL(glfwGetProcAddress)) {
    Logger::Error("gladLoadGL failed");
    glfwDestroyWindow(m_Window);
    m_Window = nullptr;
    glfwTerminate();
    return;
  }

  m_Camera.SetViewport(m_Config.width, m_Config.height);
  m_Camera.SetOrthoSize(10.0f);

  if (m_Config.enableImGui) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_Window, /*install_callbacks=*/true);
    ImGui_ImplOpenGL3_Init("#version 330");
    m_ImguiInited = true;
  }

  Logger::Info("GL: %s",
               reinterpret_cast<const char *>(glGetString(GL_VERSION)));
}

Game::~Game() {
  if (m_ImguiInited) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }

  if (m_Window) {
    glfwDestroyWindow(m_Window);
  }
  glfwTerminate();
}

bool Game::Run() {
  if (!m_Window) {
    Logger::Error("Game::Run called without a valid window");
    return false;
  }

  OnInitialize();

  m_Running = true;
  double last = glfwGetTime();
  while (m_Running && !glfwWindowShouldClose(m_Window)) {
    glfwPollEvents();

    const double now = glfwGetTime();
    const double dt = now - last;
    last = now;

    OnUpdate(dt);
    OnRender();

    if (m_Config.showFps) {
      m_FpsAccum += dt;
      m_FpsFrames += 1;
      if (m_FpsAccum >= 0.5) {
        const double fps = m_FpsFrames / m_FpsAccum;
        const double ms = 1000.0 / (fps > 1e-3 ? fps : 1.0);
        char buf[160];
        std::snprintf(buf, sizeof(buf), "%s  |  %.0f FPS  |  %.2f ms",
                      m_Config.title.c_str(), fps, ms);
        glfwSetWindowTitle(m_Window, buf);
        m_FpsAccum = 0.0;
        m_FpsFrames = 0;
      }
    }

    if (m_ImguiInited) {
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      OnImGui();

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    glfwSwapBuffers(m_Window);
  }

  OnShutdown();

  return true;
}

bool Game::IsKeyDown(int key) const {
  if (key < 0 || key > GLFW_KEY_LAST)
    return false;
  return m_Keys[static_cast<std::size_t>(key)];
}

void Game::ThunkKey(GLFWwindow *window, int key, int scancode, int action,
                    int mods) {
  Game *self = FromWindow(window);
  if (!self)
    return;

  if (key >= 0 && key <= GLFW_KEY_LAST) {
    if (action == GLFW_PRESS)
      self->m_Keys[key] = true;
    else if (action == GLFW_RELEASE)
      self->m_Keys[key] = false;
  }

  self->OnKey(key, scancode, action, mods);
}

void Game::ThunkMouseButton(GLFWwindow *window, int button, int action,
                            int mods) {
  if (Game *self = FromWindow(window))
    self->OnMouseButton(button, action, mods);
}

void Game::ThunkCursorPos(GLFWwindow *window, double x, double y) {
  if (Game *self = FromWindow(window))
    self->OnMouseMove(x, y);
}

void Game::ThunkScroll(GLFWwindow *window, double x, double y) {
  if (Game *self = FromWindow(window))
    self->OnScroll(x, y);
}

void Game::ThunkFramebufferSize(GLFWwindow *window, int width, int height) {
  if (Game *self = FromWindow(window)) {
    self->m_Camera.SetViewport(width, height);
    self->OnResize(width, height);
    glViewport(0, 0, width, height);
  }
}

} // namespace engine

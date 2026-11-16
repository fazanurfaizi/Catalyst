#pragma once

#include <cstdint>
#include <vector>

struct GLFWwindow;

namespace engine::graphics {

// arm-then-collect capture wrapper. Shape (Request -> ReadBack before Present)
// is what survives the eventual move to an explicit-present API (Vulkan).
class SwapChain {
public:
  SwapChain() = default;
  ~SwapChain();
  SwapChain(const SwapChain &) = delete;
  SwapChain &operator=(const SwapChain &) = delete;

  void Initialize(GLFWwindow *window);
  void Shutdown();

  // Arm capture for the current frame.
  void RequestCapture();

  // Collect pixels from the backbuffer. Must be called before Present().
  // Returns true on success; fills w/h with the framebuffer size.
  bool ReadBackCapture(std::vector<std::uint8_t> &out, int &w, int &h);

  void Present();

private:
  GLFWwindow *m_window = nullptr;
  bool m_captureRequested = false;
};

} // namespace engine::graphics

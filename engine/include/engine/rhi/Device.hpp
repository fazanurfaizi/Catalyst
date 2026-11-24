#pragma once

#include "engine/rhi/DeviceInfo.hpp"

#include <cstdint>
#include <memory>

namespace engine::rhi {

class BindGroup;
class BindGroupLayout;
class Buffer;
class CommandList;
class Pipeline;
class RenderPass;
class Sampler;
class ShaderStages;
class SwapChain;
class Texture;

class DeviceCreateInfo {
public:
  void *nativeWindow{nullptr}; // GLFWwindow*; Vulkan: needed for surface
  std::uint32_t framesInFlight{2};
  bool vsync{true};
};

class Device {
public:
  [[nodiscard]] static std::unique_ptr<Device> Create(const DeviceCreateInfo &createInfo);
  virtual ~Device() = default;

  // Frame lifecycle
  virtual bool BeginFrame() = 0;
  virtual void EndFrame() = 0;
  virtual void WaitIdle() = 0;

  [[nodiscard]] virtual std::uint64_t FrameIndex() const = 0;
  [[nodiscard]] virtual std::uint32_t FramesInFlight() const = 0;
  [[nodiscard]] virtual CommandList &Cmd() = 0;
  [[nodiscard]] virtual SwapChain &GetSwapChain() = 0;
  [[nodiscard]] virtual const DeviceInfo &Info() const = 0;

  // Resource creation
  virtual std::unique_ptr<SwapChain> CreateSwapChain(void *nativeWindow) = 0;
  virtual std::unique_ptr<Texture> CreateTexture(const class TextureCreateInfo &) = 0;
  virtual std::unique_ptr<Sampler> CreateSampler(const class SamplerCreateInfo &) = 0;
  virtual std::unique_ptr<Buffer> CreateBuffer(const class BufferCreateInfo &) = 0;
  virtual std::unique_ptr<ShaderStages> CreateShaderStages(const class ShaderStagesCreateInfo &) = 0;
  virtual std::unique_ptr<BindGroupLayout> CreateBindGroupLayout(const class BindGroupLayoutCreateInfo &) = 0;
  virtual std::unique_ptr<BindGroup> CreateBindGroup(const class BindGroupCreateInfo &) = 0;
  virtual std::unique_ptr<Pipeline> CreatePipeline(const class PipelineCreateInfo &) = 0;
  virtual std::unique_ptr<RenderPass> CreateRenderPass(const class RenderPassDescription &) = 0;

protected:
  Device() = default;
};

} // namespace engine::rhi

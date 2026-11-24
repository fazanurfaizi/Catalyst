#pragma once

#include "engine/rhi/Device.hpp"
#include "glad/gl.h"

#include <array>

namespace engine::rhi {

class GLDevice final : public Device {
public:
  bool BeginFrame() override;

  void EndFrame() override;

  void WaitIdle() override;

  [[nodiscard]] std::uint64_t FrameIndex() const override { return m_frameIndex; }

  [[nodiscard]] std::uint32_t FramesInFlight() const override { return m_framesInFlight; }

  [[nodiscard]] CommandList &Cmd() override;

  [[nodiscard]] SwapChain &GetSwapChain() override { return *m_swapChain; }

  [[nodiscard]] const DeviceInfo &Info() const override { return m_info; }

  std::unique_ptr<SwapChain> CreateSwapChain(void *nativeWindow) override;
  std::unique_ptr<Texture> CreateTexture(const class TextureCreateInfo &) override;
  std::unique_ptr<Sampler> CreateSampler(const class SamplerCreateInfo &) override;
  std::unique_ptr<Buffer> CreateBuffer(const class BufferCreateInfo &) override;
  std::unique_ptr<ShaderStages> CreateShaderStages(const class ShaderStagesCreateInfo &) override;
  std::unique_ptr<BindGroupLayout> CreateBindGroupLayout(const class BindGroupLayoutCreateInfo &) override;
  std::unique_ptr<BindGroup> CreateBindGroup(const class BindGroupCreateInfo &) override;
  std::unique_ptr<Pipeline> CreatePipeline(const class PipelineCreateInfo &) override;
  std::unique_ptr<RenderPass> CreateRenderPass(const class RenderPassDescription &) override;

private:
  std::uint64_t m_frameIndex{0};
  std::uint32_t m_framesInFlight{2};
  DeviceInfo m_info;
  // std::unique_ptr<GLCommandList> m_commandList;
  std::unique_ptr<SwapChain> m_swapChain;
  std::array<GLsync, 3> m_frameFences{};
};
} // namespace engine::rhi

#pragma once

#include "engine/rhi/Enums.hpp"
#include "engine/rhi/PipelineState.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace engine::rhi {

class BindGroupLayout;
class Device;
class PipelineImpl;
class ShaderStages;

class PipelineCreateInfo {
public:
  std::string name;
  const ShaderStages *shaderStages{nullptr};
  VertexInputLayout vertexInput;
  std::vector<const BindGroupLayout *> bindGroupLayouts; // index = set index

  ColorBlendState colorBlend;
  DepthStencilState depthStencil;
  RasterizationState rasterization;
  InputAssemblyState inputAssembly;
  MultisampleState multisample;

  // Render pass compatibilty
  std::vector<Format> colorAttachmentFormats;
  Format depthAttachmentFormat{Format::Undefined};
  std::uint32_t sampleCount{1};
};

class Pipeline {
public:
  Pipeline(Device &device, const PipelineCreateInfo &createInfo);
  ~Pipeline();

  Pipeline(const Pipeline &) = delete;
  Pipeline &operator=(const Pipeline &) = delete;

  [[nodiscard]] const std::string &Name() const;
  [[nodiscard]] PipelineImpl &GetImpl();

private:
  std::unique_ptr<PipelineImpl> m_impl;
};

} // namespace engine::rhi

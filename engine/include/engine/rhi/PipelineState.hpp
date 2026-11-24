#pragma once

#include "engine/rhi/Enums.hpp"
#include <cstdint>
#include <vector>

namespace engine::rhi {

class BlendStateComponent {
public:
  BlendEquation equation{BlendEquation::Add};
  BlendFactor source{BlendFactor::One};
  BlendFactor destination{BlendFactor::Zero};
};

class ColorBlendState {
public:
  bool enabled{false};
  BlendStateComponent rgb{};
  BlendStateComponent alpha{};
  bool writeR{true}, writeG{true}, writeB{true}, writeA{true};

  static ColorBlendState AlphaBlend();
  static ColorBlendState Disabled();
};

class DepthStencilState {
public:
  bool depthTest{false};
  bool depthWrite{true};
  CompareOp depthCompare{CompareOp::Less};
  bool stencilTest{false};

  static DepthStencilState TestDisabled();
  static DepthStencilState TestEnabled();
};

class RasterizationState {
public:
  CullMode cullMode{CullMode::None};
  FrontFace frontFace{FrontFace::CounterClockwise};
  bool scissorTest{false};
};

class InputAssemblyState {
public:
  PrimitiveTopology topology{PrimitiveTopology::TriangleList};
};

class MultisampleState {
public:
  std::uint32_t sampleCount{1};
};

enum class VertexFormat : std::uint8_t { Float, Float2, Float3, Float4, UByte4_Unorm, UShort2, UShort4 };

class VertexAttribute {
public:
  std::uint32_t location{0};
  std::uint32_t bufferBinding{0};
  VertexFormat format{VertexFormat::Float};
  std::uint32_t offset{0};
  std::uint32_t stepRate{0}; // 0 = per vertex, 1 = per instance
};

class VertexInputLayout {
public:
  std::vector<VertexAttribute> attributes;
  std::vector<std::uint32_t> strides; // indexed by bufferBinding
};

} // namespace engine::rhi

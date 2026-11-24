#pragma once

#include <cstdint>

namespace engine::rhi {

enum class Format : std::uint8_t {
  Undefined = 0,
  R8_Unorm,
  RGB8_Unorm,
  RGBA8_Unorm,
  RGBA8_Srgb,
  RGBA16_Float,
  R32_Float,
  RGBA32_Float,
  D24_Unorm_S8_Uint,
};

enum class BufferUsage : std::uint32_t {
  None = 0,
  Vertex = 1u << 0,
  Index = 1u << 1,
  Uniform = 1u << 2,
  Storage = 1u << 3,
  CpuWritable = 1u << 4, // persistent map
  GpuReadable = 1u << 5, // readback / copy destination
};
constexpr BufferUsage operator|(BufferUsage a, BufferUsage b) {
  return static_cast<BufferUsage>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

enum class BufferStorage { Data, PersistentMapped };

enum class TextureUsage : std::uint32_t {
  None = 0,
  Sampled = 1u << 0,
  ColorAttachment = 1u << 1,
  DepthAttachment = 1u << 2,
  CopySource = 1u << 3,
  CopyDestination = 1u << 4,
};

enum class ResourceState : std::uint8_t {
  Undefined,
  ColorAttachment,
  DepthAttachment,
  ShaderRead,
  CopySource,
  Present,
};

enum class PrimitiveTopology : std::uint8_t {
  TriangleList,
  TriangleStrip,
  LineList,
};
enum class IndexFormat : std::uint8_t { UInt16, UInt32 };
enum class CompareOp : std::uint8_t {
  Never,
  Less,
  Equal,
  LessEqual,
  Greater,
  NotEqual,
  GreaterEqual,
  Always
};
enum class BlendFactor : std::uint8_t {
  Zero,
  One,
  SrcAlpha,
  OneMinusSrcAlpha,
  DstAlpha,
  OneMinusDstAlpha
};
enum class BlendEquation : std::uint8_t { Add, Subtract, ReverseSubtract, Min, Max };
enum class CullMode : std::uint8_t { None, Front, Back };
enum class FrontFace : std::uint8_t { CounterClockwise, Clockwise };
enum class Filter : std::uint8_t { Nearest, Linear };
enum class AddressMode : std::uint8_t { Repeat, MirroredRepeat, ClampToEdge };

enum class ShaderStageFlags : std::uint32_t {
  None = 0,
  Vertex = 1u << 0,
  Fragment = 1u << 1,
};

[[nodiscard]] auto FormatIsDepth(Format format) -> bool;
[[nodiscard]] auto FormatHasStencil(Format format) -> bool;
[[nodiscard]] auto FormatByteSize(Format format) -> std::uint32_t;

} // namespace engine::rhi

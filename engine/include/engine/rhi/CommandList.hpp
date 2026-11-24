#pragma once

#include "engine/rhi/Enums.hpp"

#include <cstdint>
#include <span>

namespace engine::rhi {

class BindGroup;
class Buffer;
class Pipeline;
class RenderPass;
class Texture;

class CommandList {
public:
  virtual ~CommandList() = default;

  virtual void BeginRenderPass(const RenderPass &pass) = 0;
  virtual void EndRenderPass() = 0;

  virtual void SetPipeline(const Pipeline &pipeline) = 0;
  virtual void SetBindGroup(std::uint32_t set, const BindGroup &group, std::span<const std::uint32_t> dynamicOffsets = {}) = 0;

  virtual void SetVertexBuffer(std::uint32_t binding, const Buffer &buffer, std::uint64_t offset = 0) = 0;
  virtual void SetIndexBuffer(const Buffer &buffer, IndexFormat format, std::uint64_t offset = 0) = 0;

  virtual void SetViewport(std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height) = 0;
  virtual void SetScissor(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) = 0;

  virtual void Draw(std::uint32_t vertexCount, std::uint32_t firstVertex = 0) = 0;
  virtual void DrawIndexed(std::uint32_t indexCount, std::uint32_t firstIndex = 0, std::int32_t vertexOffset = 0, std::uint32_t instanceCount = 1,
                           std::uint32_t firstInstance = 0) = 0;

  virtual void Transition(Texture &texture, ResourceState before, ResourceState after) = 0;
};

} // namespace engine::rhi

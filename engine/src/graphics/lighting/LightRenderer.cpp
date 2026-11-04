#include "engine/graphics/lighting/LightRenderer.hpp"
#include "engine/graphics/lighting/LightBuffer.hpp"
#include <algorithm>
#include <glad/gl.h>

namespace engine::graphics {

namespace {
constexpr std::size_t kPointUboSize =
    sizeof(PointLightGpu) * LightBuffer::kMaxPoint;
constexpr std::size_t kDirectionalUboSize =
    sizeof(DirectionalLightGpu) * LightBuffer::kMaxDirectional;
constexpr std::size_t kSpotUboSize =
    sizeof(SpotLightGpu) * LightBuffer::kMaxSpot;

std::uint32_t CreateUbo(std::uint32_t binding, std::size_t size) {
  std::uint32_t ubo = 0;
  glGenBuffers(1, &ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, ubo);
  glBufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(size), nullptr,
               GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  return ubo;
}

void EnsureSsboCapacity(std::uint32_t &ssbo, std::size_t &capacity,
                        std::size_t needed) {
  if (needed <= capacity && ssbo != 0)
    return;
  std::size_t newCapacity = (capacity == 0) ? needed : capacity;
  while (newCapacity < needed) {
    newCapacity *= 2;
  }

  if (ssbo == 0)
    glGenBuffers(1, &ssbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
  glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(newCapacity),
               nullptr, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  capacity = newCapacity;
}

} // namespace

LightRenderer::LightRenderer() {
  m_pointUbo = CreateUbo(kPointBinding, kPointUboSize);
  m_directionalUbo = CreateUbo(kDirectionalBinding, kDirectionalUboSize);
  m_spotUbo = CreateUbo(kSpotBinding, kSpotUboSize);
}

LightRenderer::~LightRenderer() {
  if (m_pointUbo)
    glDeleteBuffers(1, &m_pointUbo);
  if (m_directionalUbo)
    glDeleteBuffers(1, &m_directionalUbo);
  if (m_spotUbo)
    glDeleteBuffers(1, &m_spotUbo);
  if (m_tileOffsetSsbo)
    glDeleteBuffers(1, &m_tileOffsetSsbo);
  if (m_tileIndexSsbo)
    glDeleteBuffers(1, &m_tileIndexSsbo);
}

void LightRenderer::Upload(const LightBuffer &buf) {
  glBindBufferBase(GL_UNIFORM_BUFFER, kPointBinding, m_pointUbo);
  glBindBufferBase(GL_UNIFORM_BUFFER, kDirectionalBinding, m_directionalUbo);
  glBindBufferBase(GL_UNIFORM_BUFFER, kSpotBinding, m_spotUbo);

  auto upload = [](std::uint32_t ubo, int count, auto *src,
                   std::size_t stride) {
    if (count <= 0)
      return;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(
        GL_UNIFORM_BUFFER, 0,
        static_cast<GLsizeiptr>(stride * static_cast<std::size_t>(count)), src);
  };

  upload(m_pointUbo, buf.pointCount, buf.points, sizeof(PointLightGpu));
  upload(m_directionalUbo, buf.directionalCount, buf.directional,
         sizeof(DirectionalLightGpu));
  upload(m_spotUbo, buf.spotCount, buf.spots, sizeof(SpotLightGpu));

  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void LightRenderer::UploadTileGrid(const std::uint32_t *offsets,
                                   std::size_t offsetCount,
                                   const std::uint32_t *indices,
                                   std::size_t indexCount) {
  EnsureSsboCapacity(m_tileOffsetSsbo, m_tileOffsetCap,
                     offsetCount * sizeof(std::uint32_t));
  EnsureSsboCapacity(m_tileIndexSsbo, m_tileIndexCap,
                     std::max<std::size_t>(1, indexCount) *
                         sizeof(std::uint32_t));

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_tileOffsetSsbo);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
                  static_cast<GLsizeiptr>(offsetCount * sizeof(std::uint32_t)),
                  offsets);

  if (indexCount > 0) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_tileIndexSsbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
                    static_cast<GLsizeiptr>(indexCount * sizeof(std::uint32_t)),
                    indices);
  }

  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kTileOffsetBinding,
                   m_tileOffsetSsbo);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, kTileIndexBinding,
                   m_tileIndexSsbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

} // namespace engine::graphics

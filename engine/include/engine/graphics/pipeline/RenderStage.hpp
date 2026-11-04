#pragma once

#include <cstdint>

namespace engine::graphics {

enum class RenderStage : std::uint8_t {
  Sprite = 0,
  Light = 1,
  Composite = 2,
  Blit = 3,
  ClearDepth = 4,
};

enum Attachment : std::uint32_t {
  AttachNone = 0u,
  AttachSceneColor = 1u << 0,
  AttachSceneDepth = 1u << 1,
  AttachLightColor = 1u << 2,
  AttachScreen = 1u << 3,
};

struct StageIO {
  std::uint32_t requiresMask;
  std::uint32_t produces;
};

constexpr StageIO StageInfoOf(RenderStage s) noexcept {
  switch (s) {
  case RenderStage::Sprite:
    return {AttachNone, AttachSceneColor | AttachSceneDepth}; // produces 0x3
  case RenderStage::Light:
    return {AttachSceneDepth, AttachLightColor}; // produces 0x4
  case RenderStage::Composite:
    return {AttachSceneColor | AttachLightColor, AttachScreen}; // requires 0x5
  case RenderStage::Blit:
    return {AttachNone, AttachScreen};
  case RenderStage::ClearDepth:
    return {AttachNone, AttachSceneDepth};
  }
  return {AttachNone, AttachNone};
}

inline const char *StageName(RenderStage s) noexcept {
  switch (s) {
  case RenderStage::Sprite:
    return "Sprite";
  case RenderStage::Light:
    return "Light";
  case RenderStage::Composite:
    return "Composite";
  case RenderStage::Blit:
    return "Blit";
  case RenderStage::ClearDepth:
    return "ClearDepth";
  }
  return "Unknown";
}

} // namespace engine::graphics

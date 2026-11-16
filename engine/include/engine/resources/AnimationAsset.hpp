#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetType.hpp"
#include <cstdint>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <variant>

namespace engine::resources {

enum class EaseType : std::uint8_t {
  None = 0,
};

// clang-format off
enum class TrackProperty : std::uint16_t {
  None = 0,
  // Transform
  Position, PositionX, PositionY, PositionZ,
  Rotation, Scale, ScaleX, ScaleY,
  // Sprite
  Color, ColorR, ColorG, ColorB, ColorA,
  // Camera
  CameraSize,
};
// clang-format on

extern const std::map<std::string, TrackProperty> StringToTrackProperty;

using KeyframeValue = std::variant<float, int, bool, std::string, glm::vec2,
                                   glm::vec3, glm::vec4>;

struct Keyframe {
  float time = 0.0f;
  KeyframeValue value = 0.0f;

  void InitializeZero(TrackProperty property);
};

struct Track {
  TrackProperty property;
  std::vector<Keyframe> keyframes;

  void SortKeyframes();
};

class AnimationAsset : public asset::Asset {
public:
  explicit AnimationAsset(asset::AssetHandle handle) : asset::Asset(handle) {}

  static asset::AssetType StaticType() { return asset::AssetType::Animation; }
  asset::AssetType Type() const override { return StaticType(); }

  void SortKeyframes();

  static const char *TrackPropertyToString(TrackProperty property) noexcept;
  static TrackProperty TrackPropertyFromString(const std::string &str) noexcept;
  static float Ease(float t, EaseType easingFunction = EaseType::None) noexcept;
  static KeyframeValue Lerp(KeyframeValue a, KeyframeValue b, float t);

public:
  std::vector<Track> tracks = {};
  std::string name;
  float duration = 0.0f;
  bool loop = false;
};

} // namespace engine::resources

#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetType.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/ecs/Registry.hpp"
#include <algorithm>
#include <cstdint>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <variant>

namespace engine::graphics {

enum class Ease {
  None = 0,
};

enum class TrackProperty : std::uint16_t {
  None = 0,
  Position,
  Rotation,
  Scale,
  Color,
  PositionX,
  PositionY,
  PositionZ,
  ScaleX,
  ScaleY,
  ColorR,
  ColorG,
  ColorB,
  ColorA,
  CameraSize,
};

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

  void SortKeyframes() {
    std::sort(
        keyframes.begin(), keyframes.end(),
        [](const Keyframe &a, const Keyframe &b) { return a.time < b.time; });
  }
};

class AnimationAsset : public asset::Asset {
public:
  explicit AnimationAsset(asset::AssetHandle handle) : asset::Asset(handle) {}

  static asset::AssetType StaticType() { return asset::AssetType::Animation; }
  asset::AssetType Type() const override { return StaticType(); }

  void SortKeyframes() {
    for (Track &track : tracks) {
      track.SortKeyframes();
    }
  }

  static const std::string TrackPropertyToString(TrackProperty property);
  static TrackProperty TrackPropertyFromString(const std::string &str);

  void Update(ecs::Registry &registry, ecs::Entity &entity, float time) const;
  static float ease(float t, Ease easingFunction = Ease::None);
  static KeyframeValue lerp(KeyframeValue a, KeyframeValue b, float t);

  std::vector<Track> tracks = {};
  std::string name;
  float duration = 0.0f;
  bool loop = false;

private:
  static bool HasProperty(ecs::Registry &registry, ecs::Entity &entity,
                          const TrackProperty &property);
  static void ApplyPropertyValue(ecs::Registry &reg, ecs::Entity &entity,
                                 const TrackProperty &property,
                                 const KeyframeValue &value);
};

} // namespace engine::graphics

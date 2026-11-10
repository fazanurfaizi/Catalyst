#include "engine/graphics/assets/AnimationAsset.hpp"
#include "engine/components/Camera2D.hpp"
#include "engine/components/Sprite.hpp"
#include "engine/components/Transform.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/math/Color.hpp"
#include "engine/utils/Logger.hpp"
#include <variant>

namespace engine::graphics {

const std::map<std::string, TrackProperty> StringToTrackProperty = {
    {"Position", TrackProperty::Position},
    {"Rotation", TrackProperty::Rotation},
    {"Scale", TrackProperty::Scale},
    {"Color", TrackProperty::Color},
    {"PositionX", TrackProperty::PositionX},
    {"PositionY", TrackProperty::PositionY},
    {"PositionZ", TrackProperty::PositionZ},
    {"ScaleX", TrackProperty::ScaleX},
    {"ScaleY", TrackProperty::ScaleY},
    {"ColorR", TrackProperty::ColorR},
    {"ColorG", TrackProperty::ColorG},
    {"ColorB", TrackProperty::ColorB},
    {"ColorA", TrackProperty::ColorA},
    {"CameraSize", TrackProperty::CameraSize},
};

void Keyframe::InitializeZero(TrackProperty property) {
  switch (property) {
  case TrackProperty::None:
    break;
  case TrackProperty::PositionX:
  case TrackProperty::PositionY:
  case TrackProperty::PositionZ:
  case TrackProperty::ScaleX:
  case TrackProperty::ScaleY:
  case TrackProperty::ColorR:
  case TrackProperty::ColorG:
  case TrackProperty::ColorB:
  case TrackProperty::ColorA:
  case TrackProperty::Rotation:
  case TrackProperty::CameraSize:
    value = 0.0f;
    break;
  case TrackProperty::Position:
  case TrackProperty::Scale:
    value = glm::vec3(0);
    break;
  case TrackProperty::Color:
    value = glm::vec4(0);
    break;
  }
}

const std::string
AnimationAsset::TrackPropertyToString(TrackProperty property) {
  for (const auto &pair : StringToTrackProperty) {
    if (pair.second == property) {
      return pair.first;
    }
  }
  return "Unknown";
}

TrackProperty AnimationAsset::TrackPropertyFromString(const std::string &str) {
  auto it = StringToTrackProperty.find(str);
  if (it != StringToTrackProperty.end()) {
    return it->second;
  }
  return TrackProperty::None;
}

bool AnimationAsset::HasProperty(ecs::Registry &reg, ecs::Entity &entity,
                                 const TrackProperty &property) {
  switch (property) {
  case TrackProperty::None:
  case TrackProperty::Position:
  case TrackProperty::Rotation:
  case TrackProperty::Scale:
  case TrackProperty::PositionX:
  case TrackProperty::PositionY:
  case TrackProperty::PositionZ:
  case TrackProperty::ScaleX:
  case TrackProperty::ScaleY:
    return (reg.Has<components::Transform>(entity));
  case TrackProperty::Color:
  case TrackProperty::ColorR:
  case TrackProperty::ColorG:
  case TrackProperty::ColorB:
  case TrackProperty::ColorA:
    return (reg.Has<components::Sprite>(entity));
  case TrackProperty::CameraSize:
    return (reg.Has<components::Camera2D>(entity));
  }
  return false;
}

void AnimationAsset::ApplyPropertyValue(ecs::Registry &reg, ecs::Entity &entity,
                                        const TrackProperty &property,
                                        const KeyframeValue &value) {
  auto &transform = reg.Get<components::Transform>(entity);
  components::Sprite *sprite = reg.TryGet<components::Sprite>(entity);

  switch (property) {
  case TrackProperty::None:
    break;
  case TrackProperty::Position:
    transform.position = std::get<glm::vec2>(value);
    break;
  case TrackProperty::PositionX:
    transform.position.x = std::get<float>(value);
    break;
  case TrackProperty::PositionY:
  case TrackProperty::PositionZ:
    transform.position.y = std::get<float>(value);
    break;
    // case TrackProperty::PositionZ:
    //   transform.position.z = std::get<float>(value);
    //   break;

    // TODO: quat
    // 		case TrackProperty::Rotation: transform.LocalRotation =
    // std::get<glm::quat>(value); break;
  case TrackProperty::Rotation:
    break;

  case TrackProperty::Scale:
    transform.scale = std::get<glm::vec2>(value);
    break;
  case TrackProperty::ScaleX:
    transform.scale.x = std::get<float>(value);
    break;
  case TrackProperty::ScaleY:
    transform.scale.y = std::get<float>(value);
    break;

  case TrackProperty::Color:
    if (!sprite) {
      break;
    }
    sprite->color = math::Color::fromVec4(std::get<glm::vec4>(value));
    break;
  case TrackProperty::ColorR:
    if (!sprite) {
      break;
    }
    sprite->color.r = std::get<float>(value);
    break;
  case TrackProperty::ColorG:
    if (!sprite) {
      break;
    }
    sprite->color.g = std::get<float>(value);
    break;
  case TrackProperty::ColorB:
    if (!sprite) {
      break;
    }
    sprite->color.b = std::get<float>(value);
    break;
  case TrackProperty::ColorA:
    if (!sprite) {
      break;
    }
    sprite->color.a = std::get<float>(value);
    break;

  case TrackProperty::CameraSize:
    reg.Get<components::Camera2D>(entity).SetZoom(std::get<float>(value));
    break;
  }
}

float AnimationAsset::ease(float t, Ease easing_function) {
  switch (easing_function) {
  case Ease::None:
    return t;
  }
  Logger::Error("[AnimationAsset] ease function unreachable");
  return t;
}

KeyframeValue AnimationAsset::lerp(KeyframeValue a, KeyframeValue b, float t) {
  if (std::holds_alternative<float>(a) && std::holds_alternative<float>(b)) {
    return std::get<float>(a) + t * (std::get<float>(b) - std::get<float>(a));
  }

  if (std::holds_alternative<int>(a) && std::holds_alternative<int>(b)) {
    float a_float = static_cast<float>(std::get<int>(a));
    float b_float = static_cast<float>(std::get<int>(b));
    return a_float + t * (b_float - a_float);
  }

  if (std::holds_alternative<bool>(a) && std::holds_alternative<bool>(b)) {
    return t >= 1.0f ? std::get<bool>(b) : std::get<bool>(a);
  }

  Logger::Error("[AnimationAsset] Unsupported type for lerp operation");
  return 0;
}

void AnimationAsset::Update(ecs::Registry &reg, ecs::Entity &entity,
                            float time) const {
  for (const auto &track : tracks) {
    if (!HasProperty(reg, entity, track.property)) {
      continue;
    }

    const Keyframe *curr_kf = nullptr;
    const Keyframe *next_kf = nullptr;

    for (size_t i = 0; i < track.keyframes.size() - 1; ++i) {
      if (time >= track.keyframes[i].time &&
          time < track.keyframes[i + 1].time) {
        curr_kf = &track.keyframes[i];
        next_kf = &track.keyframes[i + 1];
        break;
      }
    }

    if (!curr_kf || !next_kf) {
      continue;
    }

    float track_duration = next_kf->time - curr_kf->time;
    float t = (time - curr_kf->time) / track_duration;

    // TODO: easing functions
    t = AnimationAsset::ease(t);

    auto interpolated_value =
        AnimationAsset::lerp(curr_kf->value, next_kf->value, t);
    ApplyPropertyValue(reg, entity, track.property, interpolated_value);
  }
}

} // namespace engine::graphics

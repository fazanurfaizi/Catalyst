#include "engine/resources/AnimationAsset.hpp"

#include <algorithm>

namespace engine::resources {

const std::map<std::string, TrackProperty> StringToTrackProperty = {
    {"Position", TrackProperty::Position},
    {"PositionX", TrackProperty::PositionX},
    {"PositionY", TrackProperty::PositionY},
    {"PositionZ", TrackProperty::PositionZ},
    {"Rotation", TrackProperty::Rotation},
    {"Scale", TrackProperty::Scale},
    {"ScaleX", TrackProperty::ScaleX},
    {"ScaleY", TrackProperty::ScaleY},
    {"Color", TrackProperty::Color},
    {"ColorR", TrackProperty::ColorR},
    {"ColorG", TrackProperty::ColorG},
    {"ColorB", TrackProperty::ColorB},
    {"ColorA", TrackProperty::ColorA},
    {"CameraSize", TrackProperty::CameraSize},
};

void Track::SortKeyframes() {
  std::sort(
      keyframes.begin(), keyframes.end(),
      [](const Keyframe &a, const Keyframe &b) { return a.time < b.time; });
}

void AnimationAsset::SortKeyframes() {
  for (Track &track : tracks) {
    track.SortKeyframes();
  }
}

void Keyframe::InitializeZero(TrackProperty property) {
  switch (property) {
  case TrackProperty::Position:
  case TrackProperty::Scale:
    value = glm::vec2(0.0f);
    break;
  case TrackProperty::Color:
    value = glm::vec4(0.0f);
    break;
  case TrackProperty::Rotation:
  case TrackProperty::CameraSize:
    value = 0.0f;
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
    value = 0.0f;
    break;
  case TrackProperty::None:
    break;
  }
}

const char *AnimationAsset::TrackPropertyToString(TrackProperty tp) noexcept {
  for (const auto &[name, prop] : StringToTrackProperty) {
    if (prop == tp)
      return name.c_str();
  }
  return "None";
}

TrackProperty
AnimationAsset::TrackPropertyFromString(const std::string &str) noexcept {
  auto it = StringToTrackProperty.find(str);
  return it == StringToTrackProperty.end() ? TrackProperty::None : it->second;
}

float AnimationAsset::Ease(float t, EaseType easeFunction) noexcept {
  return t;
}
KeyframeValue AnimationAsset::Lerp(KeyframeValue a, KeyframeValue b, float t) {
  if (auto pa = std::get_if<float>(&a)) {
    if (auto pb = std::get_if<float>(&b))
      return *pa + (*pb - *pa) * t;
  }
  if (auto pa = std::get_if<glm::vec2>(&a)) {
    if (auto pb = std::get_if<glm::vec2>(&b))
      return glm::mix(*pa, *pb, t);
  }
  if (auto pa = std::get_if<glm::vec3>(&a)) {
    if (auto pb = std::get_if<glm::vec3>(&b))
      return glm::mix(*pa, *pb, t);
  }
  if (auto pa = std::get_if<glm::vec4>(&a)) {
    if (auto pb = std::get_if<glm::vec4>(&b))
      return glm::mix(*pa, *pb, t);
  }
  return a;
}

} // namespace engine::resources

#include "engine/asset/finalizer/AnimationFinalizer.hpp"
#include "engine/asset/importer/animation/AnimationImportResult.hpp"
#include "engine/resources/AnimationAsset.hpp"
#include <memory>
#include <utility>
#include <variant>

namespace engine::asset {

namespace {
resources::Keyframe MakeKeyframe(const ParsedKeyframe &src) {
  resources::Keyframe dst;
  dst.time = src.time;
  dst.value = std::visit(
      [](auto &&v) -> resources::KeyframeValue {
        return resources::KeyframeValue{std::forward<decltype(v)>(v)};
      },
      src.value);
  return dst;
}
} // namespace

std::shared_ptr<Asset>
AnimationFinalizer::Finalize(AssetHandle handle,
                             const AnimationImportResult &data) {
  std::shared_ptr<resources::AnimationAsset> animation =
      std::make_shared<resources::AnimationAsset>(handle);
  animation->name = data.name;
  animation->duration = data.duration;
  animation->tracks.reserve(data.tracks.size());

  for (const ParsedTrack &srcTrack : data.tracks) {
    resources::Track track;
    track.property = resources::AnimationAsset::TrackPropertyFromString(
        srcTrack.propertyName);
    track.keyframes.reserve(srcTrack.keyframes.size());
    for (const ParsedKeyframe &srcKf : srcTrack.keyframes) {
      track.keyframes.emplace_back(MakeKeyframe(srcKf));
    }
    animation->tracks.emplace_back(std::move(track));
  }

  animation->SortKeyframes();

  return std::static_pointer_cast<Asset>(std::move(animation));
}

} // namespace engine::asset

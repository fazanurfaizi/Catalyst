#pragma once

#include "engine/asset/manager/IAssetManager.hpp"
#include "engine/audio/AudioEngine.hpp"
#include "engine/components/AudioSource.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/resources/SoundAsset.hpp"

namespace engine::systems {

inline void AudioSystem(ecs::Registry &reg, asset::IAssetManager &assets) {
  for (auto [e, src] : reg.View<components::AudioSource>()) {
    const bool wantPlay =
        src.trigger || (src.playOnSpawn && !src.hasPlayedOnSpawn);
    if (!wantPlay)
      continue;

    src.trigger = false;
    if (src.playOnSpawn)
      src.hasPlayedOnSpawn = true;

    if (src.clip.IsNull())
      continue;

    auto sound = assets.Acquire<resources::SoundAsset>(src.clip);
    if (!sound)
      continue;

    audio::AudioEngine::Get().PlayOneShot(*sound, src.volume);
  }
}

inline void PlayAudioNow(ecs::Registry &reg, asset::IAssetManager &assets,
                         ecs::Entity e) {
  auto *src = reg.TryGet<components::AudioSource>(e);
  if (!src || src->clip.IsNull())
    return;

  auto sound = assets.Acquire<resources::SoundAsset>(src->clip);
  if (!sound)
    return;

  audio::AudioEngine::Get().PlayOneShot(*sound, src->volume);
}

} // namespace engine::systems

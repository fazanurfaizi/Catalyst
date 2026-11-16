#include "test_harness.hpp"

#include "engine/audio/AudioEngine.hpp"
#include "engine/resources/SoundAsset.hpp"

void RunAudioEngineTests() {
  using engine::audio::AudioEngine;
  using engine::resources::SoundAsset;

  auto &audio = AudioEngine::Get();

  CHECK(!audio.IsInitialized());

  SoundAsset sound(engine::asset::AssetHandle::FromString("dddd000000000001"));
  CHECK(sound.IsEmpty());

  audio.PlayOneShot(sound);
  audio.SetMasterVolume(0.5f);
  audio.Update();
  audio.Shutdown();

  CHECK(true);
}

#include "engine/audio/AudioEngine.hpp"
#include "engine/resources/SoundAsset.hpp"
#include "engine/utils/Logger.hpp"
#include <mutex>
#include <vector>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace engine::audio {

struct AudioEngine::Impl {
  ma_engine engine{};
  bool engineInited = false;

  std::mutex finishedMutex;
  std::vector<ma_sound *> finishedSounds;
};

AudioEngine &AudioEngine::Get() {
  static AudioEngine instance;
  return instance;
}

AudioEngine::AudioEngine() : m_impl(new Impl()) {}

AudioEngine::~AudioEngine() {
  Shutdown();
  delete m_impl;
}

bool AudioEngine::Init() {
  if (m_impl->engineInited)
    return true;
  if (ma_engine_init(nullptr, &m_impl->engine) != MA_SUCCESS) {
    Logger::Error("[AudioEngine] Failed to initialize audio engine");
    return false;
  }
  m_impl->engineInited = true;
  return true;
}

void AudioEngine::Update() {
  if (!m_impl->engineInited)
    return;

  std::vector<ma_sound *> toFree;
  {
    std::lock_guard<std::mutex> lock(m_impl->finishedMutex);
    toFree.swap(m_impl->finishedSounds);
  }

  for (auto *sound : toFree) {
    ma_sound_uninit(sound);
    delete sound;
  }
}

void AudioEngine::Shutdown() {
  if (!m_impl->engineInited)
    return;
  Update();
  ma_engine_uninit(&m_impl->engine);
  m_impl->engineInited = false;
}

bool AudioEngine::IsInitialized() const { return m_impl->engineInited; }

void AudioEngine::PlayOneShot(const resources::SoundAsset &sound,
                              float volume) {
  if (!m_impl->engineInited) {
    Logger::Warn("[AudioEngine] PlayOneShot before Init, ignoring");
    std::abort();
    return;
  }

  if (sound.sourcePath.empty()) {
    Logger::Warn("[AudioEngine] PlayOneShot: sound has no source path");
    return;
  }

  Update();

  auto *instance = new ma_sound();
  if (ma_sound_init_from_file(
          &m_impl->engine, sound.sourcePath.string().c_str(),
          MA_SOUND_FLAG_DECODE, nullptr, nullptr, instance) != MA_SUCCESS) {
    Logger::Error("[AudioEngine] Failed to init sound from '%s'",
                  sound.sourcePath.string().c_str());
    delete instance;
    return;
  }

  ma_sound_set_volume(instance, volume);
  ma_sound_set_end_callback(instance, OnSoundFinished, m_impl);
  ma_sound_start(instance);
}

void AudioEngine::PlaySoundPath(const std::string &filepath, float volume) {
  if (!m_impl->engineInited)
    return;

  auto *instance = new ma_sound();
  if (ma_sound_init_from_file(&m_impl->engine, filepath.c_str(),
                              MA_SOUND_FLAG_DECODE, nullptr, nullptr,
                              instance) != MA_SUCCESS) {
    delete instance;
    return;
  }

  ma_sound_set_volume(instance, volume);
  ma_sound_set_end_callback(instance, OnSoundFinished, m_impl);
  ma_sound_start(instance);
}

void AudioEngine::SetMasterVolume(float volume) {
  if (!m_impl->engineInited)
    return;
  ma_engine_set_volume(&m_impl->engine, volume);
}

ma_engine *AudioEngine::GetHandle() const {
  return m_impl->engineInited ? &m_impl->engine : nullptr;
}

void AudioEngine::OnSoundFinished(void *data, ma_sound *sound) {
  auto *impl = static_cast<AudioEngine::Impl *>(data);
  std::lock_guard<std::mutex> lock(impl->finishedMutex);
  impl->finishedSounds.push_back(sound);
}

} // namespace engine::audio

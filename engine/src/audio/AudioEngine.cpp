#include "engine/audio/AudioEngine.hpp"
#include "engine/utils/Logger.hpp"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace engine::audio {

namespace {
void OnSoundFinished([[maybe_unused]] void *data, ma_sound *sound) {
  ma_sound_uninit(sound);
  delete sound;
}
} // namespace

struct AudioEngine::Impl {
  ma_engine engine{};
  bool engineInited = false;
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

void AudioEngine::Shutdown() {
  if (!m_impl->engineInited)
    return;
  ma_engine_uninit(&m_impl->engine);
  m_impl->engineInited = false;
}

bool AudioEngine::IsInitialized() const { return m_impl->engineInited; }

void AudioEngine::PlayOneShot(const SoundAsset &sound, float volume) {
  if (!m_impl->engineInited) {
    Logger::Warn("[AudioEngine] PlayOneShot before Init, ignoring");
    return;
  }

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
  ma_sound_set_end_callback(instance, OnSoundFinished, nullptr);
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
  ma_sound_set_end_callback(instance, OnSoundFinished, nullptr);
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

} // namespace engine::audio

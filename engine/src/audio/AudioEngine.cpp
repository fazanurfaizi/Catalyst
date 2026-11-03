#include "engine/audio/AudioEngine.hpp"
#include "engine/utils/Logger.hpp"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace engine::audio {

struct AudioEngine::Impl {
  ma_engine engine{};
  ma_sound music{};
  bool engineInited = false;
  bool musicLoaded = false;
};

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
  if (m_impl->musicLoaded) {
    ma_sound_uninit(&m_impl->music);
    m_impl->musicLoaded = false;
  }
  if (m_impl->engineInited) {
    ma_engine_uninit(&m_impl->engine);
    m_impl->engineInited = false;
  }
}

void AudioEngine::PlaySound(const std::string &filepath) {
  ma_engine_play_sound(&m_impl->engine, filepath.c_str(), nullptr);
}

bool AudioEngine::PlayMusic(const std::string &filepath) {
  StopMusic();
  if (ma_sound_init_from_file(&m_impl->engine, filepath.c_str(),
                              MA_SOUND_FLAG_STREAM, nullptr, nullptr,
                              &m_impl->music) != MA_SUCCESS) {
    Logger::Error("[AudioEngine] Failed to load music: %s", filepath.c_str());
    return false;
  }
  ma_sound_set_looping(&m_impl->music, MA_TRUE);
  ma_sound_start(&m_impl->music);
  m_impl->musicLoaded = true;
  return true;
}

void AudioEngine::StopMusic() {
  if (m_impl->musicLoaded) {
    ma_sound_stop(&m_impl->music);
    ma_sound_uninit(&m_impl->music);
    m_impl->musicLoaded = false;
  }
}

void AudioEngine::SetMasterVolume(float volume) {
  ma_engine_set_volume(&m_impl->engine, volume);
}

} // namespace engine::audio

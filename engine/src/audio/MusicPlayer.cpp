#include "engine/audio/MusicPlayer.hpp"
#include "engine/audio/AudioEngine.hpp"
#include "engine/utils/Logger.hpp"

#include <miniaudio.h>

namespace engine::audio {

MusicPlayer::MusicPlayer() : m_sound(new ma_sound()) {}

MusicPlayer::~MusicPlayer() {
  if (m_loaded) {
    ma_sound_stop(m_sound);
    ma_sound_uninit(m_sound);
  }
  delete m_sound;
}

bool MusicPlayer::Load(const std::string &path) {
  ma_engine *engine = AudioEngine::Get().GetHandle();
  if (!engine) {
    Logger::Error("[MusicPlayer] Load() called before AudioEngine::Init()");
    return false;
  }

  if (m_loaded) {
    ma_sound_stop(m_sound);
    ma_sound_uninit(m_sound);
    m_loaded = false;
  }

  if (ma_sound_init_from_file(engine, path.c_str(), MA_SOUND_FLAG_STREAM,
                              nullptr, nullptr, m_sound) != MA_SUCCESS) {
    return false;
  }
  m_loaded = true;

  return true;
}

void MusicPlayer::Play() {
  if (m_loaded)
    ma_sound_start(m_sound);
}

void MusicPlayer::Stop() {
  if (m_loaded)
    ma_sound_stop(m_sound);
}

void MusicPlayer::SetVolume(float v) {
  if (m_loaded)
    ma_sound_set_volume(m_sound, v);
}

void MusicPlayer::SetLooping(bool loop) {
  if (m_loaded)
    ma_sound_set_looping(m_sound, loop ? MA_TRUE : MA_FALSE);
}

} // namespace engine::audio

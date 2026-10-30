#pragma once

#include <chrono>
#include <cstdint>

namespace engine::core {

class Clock {
public:
  explicit Clock(double fixedTimestep = 1.0 / 60.0)
      : m_fixedTimestep(fixedTimestep),
        m_previousTime(std::chrono::steady_clock::now()), m_accumulator(0.0),
        m_deltaTime(0.0), m_timeScale(1.0), m_frameCount(0) {}

  /**
   * @brief Call at the very beginning of every frame
   */
  void Tick() {
    auto currentTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = currentTime - m_previousTime;
    m_previousTime = currentTime;

    double frameTime = elapsed.count();

    // prevent Spiral of Death
    if (frameTime > 0.25) {
      frameTime = 0.25;
    }

    // Apply global time scale
    m_deltaTime = frameTime * m_timeScale;
    m_accumulator += m_deltaTime;
    m_frameCount++;
  }

  /**
   * @brief Check if enough time has built up for a fixed physics/logic step
   */
  bool ShouldConsumeStep() const { return m_accumulator >= m_fixedTimestep; }

  /**
   * @brief Consume one fixed interval from the accumulator
   */
  void ConsumeStep() { m_accumulator -= m_fixedTimestep; }

  /**
   * @brief Returns alpha factor for render interpolation (0.0 to 1.0+)
   */
  double GetInterpolationAlpha() const {
    return m_accumulator / m_fixedTimestep;
  }

  double DeltaTime() const { return m_deltaTime; }
  double FixedDeltaTime() const { return m_fixedTimestep; }

  void SetTimeScale(double scale) { m_timeScale = scale; }
  double TimeScale() const { return m_timeScale; }

  uint64_t FrameCount() const { return m_frameCount; }

private:
  double m_fixedTimestep;
  std::chrono::steady_clock::time_point m_previousTime;
  double m_accumulator;
  double m_deltaTime;
  double m_timeScale;
  std::uint64_t m_frameCount;
};

} // namespace engine::core

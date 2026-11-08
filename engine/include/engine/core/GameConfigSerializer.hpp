#pragma once

#include "engine/core/GameConfig.hpp"
#include <filesystem>
#include <optional>

namespace engine::core {

class GameConfigSerializer {
public:
  static std::optional<GameConfig> Load(const std::filesystem::path &file);
  static bool Save(const GameConfig &config, const std::filesystem::path &file);
};

} // namespace engine::core

#pragma once

#include "engine/asset/core/AssetHandle.hpp"
#include <cstdint>
#include <filesystem>

namespace engine::asset {

enum class FileEventType : std::uint8_t {
  Modified,
};

struct FileEvent {
  AssetHandle handle;
  std::filesystem::path path;
  FileEventType type = FileEventType::Modified;
};

const char *FileEventTypeToString(FileEventType type) noexcept;

} // namespace engine::asset

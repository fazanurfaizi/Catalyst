#include "engine/asset/watcher/FileEvent.hpp"

namespace engine::asset {

const char *FileEventTypeToString(FileEventType type) noexcept {
  switch (type) {
  case FileEventType::Modified:
    return "Modified";
  }
  return "Unknown";
}

} // namespace engine::asset

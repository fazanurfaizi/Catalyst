#pragma once

#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/watcher/FileEvent.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <map>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

namespace engine::asset {

class AssetWatcher {
public:
  explicit AssetWatcher(
      std::chrono::milliseconds pollInterval = std::chrono::milliseconds(500));
  ~AssetWatcher();

  AssetWatcher(const AssetWatcher &) = delete;
  AssetWatcher &operator=(const AssetWatcher &) = delete;

  void WatchFile(AssetHandle handle, std::filesystem::path path);
  void WatchDirectory(AssetHandle handle, std::filesystem::path dir);

  void Unwatch(AssetHandle handle);
  void Start();
  void Stop();

  std::vector<FileEvent> Poll();
  std::size_t WatchedCount() const;

private:
  struct WatchedFile {
    std::filesystem::path path;
    std::filesystem::file_time_type mtime;
    bool hasMtime = false;
  };

  struct WatchedDir {
    std::filesystem::path path;
    std::set<AssetHandle> handles;
    std::map<std::filesystem::path, WatchedFile> files;
  };

  static std::map<std::filesystem::path, WatchedFile>
  ScanDirectory(const std::filesystem::path &dir);

  void ThreadLoop();
  void PollOnce();

  std::chrono::milliseconds m_pollInterval;

  mutable std::mutex m_mutex;
  std::map<AssetHandle, WatchedFile> m_files;
  std::map<std::filesystem::path, WatchedDir> m_dirs;
  std::vector<FileEvent> m_pending;

  std::thread m_thread;
  std::atomic<bool> m_stop{false};
  std::atomic<bool> m_started{false};

  std::mutex m_waitMutex;
  std::condition_variable m_cv;
};

} // namespace engine::asset

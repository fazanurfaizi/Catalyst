#pragma once

#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/watcher/FileEvent.hpp"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

namespace engine::asset {

class AssetWatcher {
public:
  explicit AssetWatcher(std::chrono::milliseconds pollInterval =
                            std::chrono::milliseconds(500));
  ~AssetWatcher();

  AssetWatcher(const AssetWatcher&)            = delete;
  AssetWatcher& operator=(const AssetWatcher&) = delete;

  /** Watch a single file. Reimport fires when its mtime changes. */
  void WatchFile(AssetHandle handle, std::filesystem::path path);

  /**
   * Watch every file in a directory (recursive). Reimport fires when
   * any file beneath `dir` changes. The `handle` is reimported once
   * per poll cycle regardless of how many files changed — coalescing
   * at the watcher level avoids N reimports for an N-file edit.
   */
  void WatchDirectory(AssetHandle handle, std::filesystem::path dir);

  void Unwatch(AssetHandle handle);
  void Start();
  void Stop();
  std::vector<FileEvent> Poll();
  std::size_t WatchedCount() const;

private:
  struct WatchedFile {
    std::filesystem::path        path;
    std::filesystem::file_time_type mtime;
    bool                         hasMtime = false;
  };
  struct WatchedDir {
    std::filesystem::path path;
    // directory → last-seen mtime per file. Deleted files are pruned
    // during each poll; added files are inserted with the current mtime
    // and do not trigger a reimport on their own (only changes to
    // pre-existing files do — keeps the "no spurious reload on startup"
    // invariant from step 11.0).
    std::map<std::filesystem::path, WatchedFile> files;
  };

  void ThreadLoop();
  bool PollDirectory(WatchedDir& dir);

  std::chrono::milliseconds m_pollInterval;

  mutable std::mutex                          m_mutex;
  std::map<AssetHandle, WatchedFile>          m_files;
  std::map<AssetHandle, WatchedDir>           m_dirs;
  std::vector<FileEvent>                      m_pending;

  std::thread                                 m_thread;
  std::atomic<bool>                           m_stop{false};
  bool                                        m_started = false;
};

} // namespace engine::asset

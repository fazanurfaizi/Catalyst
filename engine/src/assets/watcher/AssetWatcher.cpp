#include "engine/asset/watcher/AssetWatcher.hpp"

#include <system_error>

namespace engine::asset {

AssetWatcher::AssetWatcher(std::chrono::milliseconds pollInterval)
    : m_pollInterval(pollInterval) {}

AssetWatcher::~AssetWatcher() { Stop(); }

void AssetWatcher::WatchFile(AssetHandle handle, std::filesystem::path path) {
  std::error_code ec;
  const auto mtime = std::filesystem::last_write_time(path, ec);

  std::lock_guard<std::mutex> lock(m_mutex);
  WatchedFile w;
  w.path = std::move(path);
  w.mtime = mtime;
  w.hasMtime = !ec;
  m_files.emplace(handle, std::move(w));
}

void AssetWatcher::WatchDirectory(AssetHandle handle,
                                  std::filesystem::path dir) {
  std::lock_guard<std::mutex> lock(m_mutex);

  WatchedDir wd;
  wd.path = std::move(dir);

  std::error_code ec;
  if (std::filesystem::is_directory(wd.path, ec) && !ec) {
    for (auto it = std::filesystem::recursive_directory_iterator(
             wd.path,
             std::filesystem::directory_options::skip_permission_denied, ec);
         !ec && it != std::filesystem::recursive_directory_iterator();
         it.increment(ec)) {
      if (!it->is_regular_file())
        continue;
      WatchedFile wf;
      wf.path = it->path();
      std::error_code ec2;
      wf.mtime = std::filesystem::last_write_time(wf.path, ec2);
      wf.hasMtime = !ec2;
      wd.files.emplace(wf.path, std::move(wf));
    }
  }

  m_dirs.emplace(handle, std::move(wd));
}

void AssetWatcher::Unwatch(AssetHandle handle) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_files.erase(handle);
  m_dirs.erase(handle);
}

void AssetWatcher::Start() {
  if (m_started)
    return;
  m_started = true;
  m_stop.store(false, std::memory_order_relaxed);
  m_thread = std::thread(&AssetWatcher::ThreadLoop, this);
}

void AssetWatcher::Stop() {
  if (!m_started)
    return;
  m_started = false;
  m_stop.store(true, std::memory_order_relaxed);
  if (m_thread.joinable())
    m_thread.join();
}

std::vector<FileEvent> AssetWatcher::Poll() {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<FileEvent> out = std::move(m_pending);
  m_pending.clear();
  return out;
}

std::size_t AssetWatcher::WatchedCount() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_files.size() + m_dirs.size();
}

void AssetWatcher::ThreadLoop() {
  while (!m_stop.load(std::memory_order_relaxed)) {
    std::this_thread::sleep_for(m_pollInterval);
    if (m_stop.load(std::memory_order_relaxed))
      break;

    std::lock_guard<std::mutex> lock(m_mutex);

    // Per-file watches: fire on mtime change
    for (auto &[handle, w] : m_files) {
      std::error_code ec;
      const auto mtime = std::filesystem::last_write_time(w.path, ec);
      if (ec)
        continue;
      if (!w.hasMtime) {
        w.mtime = mtime;
        w.hasMtime = true;
        continue;
      }
      if (mtime == w.mtime)
        continue;
      w.mtime = mtime;
      m_pending.push_back(FileEvent{handle, w.path, FileEventType::Modified});
    }

    // Directory watches: walk, compare, coalesce per handle
    for (auto &[handle, dir] : m_dirs) {
      bool changed = false;
      std::filesystem::path changedPath;

      std::error_code ec;
      std::map<std::filesystem::path, WatchedFile> current;

      for (auto it = std::filesystem::recursive_directory_iterator(
               dir.path,
               std::filesystem::directory_options::skip_permission_denied, ec);
           !ec && it != std::filesystem::recursive_directory_iterator();
           it.increment(ec)) {
        if (!it->is_regular_file())
          continue;
        const auto &p = it->path();
        std::error_code ec2;
        const auto mtime = std::filesystem::last_write_time(p, ec2);
        if (ec2)
          continue;
        current.emplace(p, WatchedFile{p, mtime, true});
      }

      for (const auto &[path, info] : current) {
        auto prev = dir.files.find(path);
        if (prev == dir.files.end()) {
          continue;
        }
        if (prev->second.mtime != info.mtime) {
          changed = true;
          changedPath = path;
        }
      }

      // Prune deleted files
      for (auto it = dir.files.begin(); it != dir.files.end();) {
        if (current.find(it->first) == current.end()) {
          changed = true;
          changedPath = it->first;
          it = dir.files.erase(it);
        } else {
          ++it;
        }
      }

      // Update the mtime table to the current snapshot
      dir.files = std::move(current);

      if (changed) {
        m_pending.push_back(
            FileEvent{handle, changedPath, FileEventType::Modified});
      }
    }
  }
}

} // namespace engine::asset

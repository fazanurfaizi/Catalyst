#include "engine/asset/watcher/AssetWatcher.hpp"

#include <system_error>
#include <utility>

namespace engine::asset {

AssetWatcher::AssetWatcher(std::chrono::milliseconds pollInterval)
    : m_pollInterval(pollInterval) {}

AssetWatcher::~AssetWatcher() { Stop(); }

std::map<std::filesystem::path, AssetWatcher::WatchedFile>
AssetWatcher::ScanDirectory(const std::filesystem::path &dir) {
  std::map<std::filesystem::path, WatchedFile> files;

  std::error_code ec;
  if (!std::filesystem::is_directory(dir, ec) || ec)
    return files;

  for (auto it = std::filesystem::recursive_directory_iterator(
           dir, std::filesystem::directory_options::skip_permission_denied, ec);
       !ec && it != std::filesystem::recursive_directory_iterator();
       it.increment(ec)) {
    if (!it->is_regular_file())
      continue;

    const auto &path = it->path();
    std::error_code ec2;
    const auto mtime = std::filesystem::last_write_time(path, ec2);
    if (ec2)
      continue;

    files.emplace(path, WatchedFile{path, mtime, true});
  }

  return files;
}

void AssetWatcher::WatchFile(AssetHandle handle, std::filesystem::path path) {
  std::error_code ec;
  const auto mtime = std::filesystem::last_write_time(path, ec);

  std::lock_guard<std::mutex> lock(m_mutex);
  WatchedFile w;
  w.path = std::move(path);
  w.mtime = mtime;
  w.hasMtime = !ec;
  m_files.insert_or_assign(handle, std::move(w));
}

void AssetWatcher::WatchDirectory(AssetHandle handle,
                                  std::filesystem::path dir) {
  std::error_code ec;
  auto canonical = std::filesystem::weakly_canonical(dir, ec);
  if (ec)
    canonical = std::move(dir);

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_dirs.find(canonical);
    if (it != m_dirs.end()) {
      it->second.handles.insert(handle);
      return;
    }
  }

  auto files = ScanDirectory(canonical);

  std::lock_guard<std::mutex> lock(m_mutex);
  auto [it, inserted] = m_dirs.try_emplace(canonical);
  it->second.path = canonical;
  it->second.handles.insert(handle);

  if (inserted) {
    it->second.files = std::move(files);
  } else {
    for (auto &[path, file] : files) {
      it->second.files.try_emplace(path, std::move(file));
    }
  }
}

void AssetWatcher::Unwatch(AssetHandle handle) {
  std::lock_guard<std::mutex> lock(m_mutex);

  m_files.erase(handle);

  for (auto it = m_dirs.begin(); it != m_dirs.end();) {
    it->second.handles.erase(handle);
    if (it->second.handles.empty()) {
      it = m_dirs.erase(it);
    } else {
      ++it;
    }
  }
}

void AssetWatcher::Start() {
  bool expected = false;
  if (!m_started.compare_exchange_strong(expected, true))
    return;

  m_stop.store(false, std::memory_order_relaxed);
  m_thread = std::thread(&AssetWatcher::ThreadLoop, this);
}

void AssetWatcher::Stop() {
  if (!m_started.exchange(false))
    return;

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

  std::size_t count = m_files.size();
  for (const auto &[path, dir] : m_dirs)
    count += dir.handles.size();

  return count;
}

void AssetWatcher::ThreadLoop() {
  while (!m_stop.load(std::memory_order_relaxed)) {
    std::this_thread::sleep_for(m_pollInterval);
    if (m_stop.load(std::memory_order_relaxed))
      break;

    // Per-file watches
    {
      std::lock_guard<std::mutex> lock(m_mutex);

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
    }

    // Snapshot directory state, then scan without holding the lock
    struct Snapshot {
      std::filesystem::path path;
      std::map<std::filesystem::path, WatchedFile> files;
    };

    std::vector<Snapshot> snapshots;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      snapshots.reserve(m_dirs.size());

      for (const auto &[path, dir] : m_dirs)
        snapshots.push_back(Snapshot{path, dir.files});
    }

    for (auto &snapshot : snapshots) {
      auto current = ScanDirectory(snapshot.path);

      bool changed = false;
      std::filesystem::path changedPath;

      for (const auto &[path, info] : current) {
        auto prev = snapshot.files.find(path);
        if (prev == snapshot.files.end())
          continue;

        if (prev->second.mtime != info.mtime) {
          changed = true;
          changedPath = path;
          break;
        }
      }

      if (!changed) {
        for (const auto &[path, prev] : snapshot.files) {
          if (current.find(path) == current.end()) {
            changed = true;
            changedPath = path;
            break;
          }
        }
      }

      {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_dirs.find(snapshot.path);
        if (it == m_dirs.end())
          continue;

        it->second.files = std::move(current);

        if (changed) {
          for (AssetHandle handle : it->second.handles) {
            m_pending.push_back(
                FileEvent{handle, changedPath, FileEventType::Modified});
          }
        }
      }
    }
  }
}

} // namespace engine::asset

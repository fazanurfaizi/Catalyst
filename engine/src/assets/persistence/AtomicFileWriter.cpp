#include "engine/asset/persistence/AtomicFileWriter.hpp"
#include "engine/utils/Logger.hpp"

#include <fstream>
#include <ios>
#include <system_error>

#if defined(__unix__) || defined(__APPLE__)
#include <fcntl.h>
#include <unistd.h>
#elif defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace engine::asset {

namespace {

std::filesystem::path TempPathFor(const std::filesystem::path &target) {
  auto tmp = target;
  tmp += ".tmp";
  return tmp;
}

bool FsyncFile(const std::filesystem::path &path) {
#if defined(__unix__) || defined(__APPLE__)
  const int fd = ::open(path.c_str(), O_RDONLY);
  if (fd < 0)
    return false;

  const bool ok = (::fsync(fd) == 0);
  ::close(fd);
  return ok;
#else
  (void)path;
  return true;
#endif // defined (__unix__) || defined (__APPLE__)
}

bool FsyncDirectory(const std::filesystem::path &dir) {
#if defined(__unix__) || defined(__APPLE__)
  const int fd = ::open(dir.c_str(), O_RDONLY | O_DIRECTORY);
  if (fd < 0)
    return false;

  const bool ok = (::fsync(fd) == 0);
  ::close(fd);
  return ok;
#else
  (void)dir;
  return true;
#endif // defined ()
}

bool FreplaceFile(const std::filesystem::path &tmp,
                  const std::filesystem::path &target) {
#if defined(_WIN32)
  if (::MoveFileExW(tmp.c_str(), target.c_str(),
                    MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
    return true;
  }
  return false;
#else
  std::error_code ec;
  std::filesystem::rename(tmp, target, ec);
  return !ec;
#endif // defined (__WIN32)
}

} // namespace

bool AtomicFileWriter::Write(const std::filesystem::path &target,
                             std::string_view contents) {
  std::error_code ec;

  const auto dir = target.parent_path();
  if (!dir.empty() && !std::filesystem::exists(dir, ec)) {
    std::filesystem::create_directories(dir, ec);
    if (ec) {
      Logger::Error("[AtomicFileWriter] cannot create '%s': %s", dir.c_str(),
                    ec.message().c_str());
      return false;
    }
  }

  const auto tmp = TempPathFor(target);

  {
    std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
    if (!out) {
      Logger::Error("[AtomicFileWriter] cannot open '%s'", tmp.c_str());
      return false;
    }

    out.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    out.flush();

    if (!out.good()) {
      Logger::Error("[AtomicFileWriter] write failed for '%s'", tmp.c_str());
      out.close();
      std::filesystem::remove(tmp, ec);
      return false;
    }
  }

  if (!FsyncFile(tmp)) {
    Logger::Warn("[AtomicFileWriter] fsync failed for '%s' (continuing)",
                 tmp.c_str());
  }

  if (!FreplaceFile(tmp, target)) {
    Logger::Error("[AtomicFileWriter] rename failed '%s' -> '%s'", tmp.c_str(),
                  target.c_str());
    std::filesystem::remove(tmp, ec);
    return false;
  }

  if (!dir.empty()) {
    (void)FsyncDirectory(dir);
  }

  return true;
}

} // namespace engine::asset

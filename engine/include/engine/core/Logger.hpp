#pragma once

#include <string>

namespace engine {

class Logger {
public:
  enum class Level { Info, Warn, Error };

  static void Log(Level level, const std::string &msg);

  template <typename... Args>
  static void Info(const char *fmt, Args &&...args) {
    Log(Level::Info, Format(fmt, std::forward<Args>(args)...));
  }

  template <typename... Args>
  static void Warn(const char *fmt, Args &&...args) {
    Log(Level::Warn, Format(fmt, std::forward<Args>(args)...));
  }

  template <typename... Args>
  static void Error(const char *fmt, Args &&...args) {
    Log(Level::Error, Format(fmt, std::forward<Args>(args)...));
  }

private:
  template <typename... Args>
  static std::string Format(const char *fmt, Args &&...args) {
    if constexpr (sizeof...(Args) == 0) {
      return fmt;
    } else {
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#endif // defined (__GNUC__) || defined (__clang__)
      int size = std::snprintf(nullptr, 0, fmt, std::forward<Args>(args)...);
      if (size <= 0) {
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
        return "";
      }

      std::string buf(size, '\0');
      std::snprintf(&buf[0], size + 1, fmt, std::forward<Args>(args)...);

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif // defined (__GNUC__) || defined (__clang__)
      return buf;
    }
  }
};

} // namespace engine

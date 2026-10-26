#include <engine/utils/Logger.hpp>

#include <cstdio>

namespace engine {

void Logger::Log(Level level, const std::string &msg) {
  const char *tag = "INFO";
  switch (level) {
  case Level::Info:
    tag = "INFO";
    break;
  case Level::Warn:
    tag = "WARN";
    break;
  case Level::Error:
    tag = "ERROR";
    break;
  }
  std::printf("[%s] %s\n", tag, msg.c_str());
  std::fflush(stdout);
}

} // namespace engine

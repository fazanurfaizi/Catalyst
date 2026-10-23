#pragma once

#include <string>

namespace engine {

inline std::string AssetPath(const std::string &relative) {
#ifdef ASSETS_PATH
  std::string base = ASSETS_PATH;
  if (!base.empty() && base.back() != '/' && base.back() != '\\') {
    base += '/';
  }
  return base + relative;
#else
  return relative;
#endif // ASSET_PATH
}

} // namespace engine

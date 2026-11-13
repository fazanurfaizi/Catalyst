#pragma once

#include <filesystem>
#include <string_view>

namespace engine::asset {

class AtomicFileWriter {
public:
  static bool Write(const std::filesystem::path &target,
                    std::string_view contents);
};

} // namespace engine::asset

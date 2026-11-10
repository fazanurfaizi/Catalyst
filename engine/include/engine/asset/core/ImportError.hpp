#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>

namespace engine::asset {

class ImportError {
public:
  enum class Code : std::uint8_t {
    None = 0,
    FileNotFound,
    FileUnreadable,
    UnsupportedFormat,
    MalformedContent,
    DecodeFailed,
    TooLarge,
    OutOfMemory,
    Internal,
  };

  ImportError() = default;

  static ImportError Make(Code code, std::string message) {
    ImportError err;
    err.m_code = code;
    err.m_message = std::move(message);
    return err;
  }

  ImportError &WithPath(std::filesystem::path path) {
    m_path = std::move(path);
    return *this;
  }

  ImportError &WithLine(int line) {
    m_line = line;
    return *this;
  }

  Code GetCode() const noexcept { return m_code; }
  const std::string &GetMessage() const noexcept { return m_message; }
  const std::filesystem::path &GetPath() const noexcept { return m_path; }
  int GetLine() const noexcept { return m_line; }

  std::string ToString() const;

private:
  Code m_code = Code::None;
  std::string m_message;
  std::filesystem::path m_path;
  int m_line = 0;
};

const char *ImportErrorCodeToString(ImportError::Code code) noexcept;

} // namespace engine::asset

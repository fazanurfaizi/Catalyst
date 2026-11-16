#include "engine/asset/core/ImportError.hpp"

#include <sstream>

namespace engine::asset {

const char *ImportErrorCodeToString(ImportError::Code code) noexcept {
  switch (code) {
  case ImportError::Code::None:
    return "None";
  case ImportError::Code::FileNotFound:
    return "FileNotFound";
  case ImportError::Code::FileUnreadable:
    return "FileUnreadable";
  case ImportError::Code::UnsupportedFormat:
    return "UnsupportedFormat";
  case ImportError::Code::MalformedContent:
    return "MalformedContent";
  case ImportError::Code::DecodeFailed:
    return "DecodeFailed";
  case ImportError::Code::TooLarge:
    return "TooLarge";
  case ImportError::Code::OutOfMemory:
    return "OutOfMemory";
  case ImportError::Code::Internal:
    return "Internal";
  }
  return "Internal";
}

std::string ImportError::ToString() const {
  std::ostringstream out;
  out << '[' << ImportErrorCodeToString(m_code) << ']';
  if (!m_path.empty()) {
    out << ' ' << m_path.string();
    if (m_line > 0)
      out << ':' << m_line;
  }
  out << ": " << m_message;
  return out.str();
}

} // namespace engine::asset

#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
namespace engine {

struct Buffer {
  std::uint8_t *data = nullptr;
  std::uint64_t size = 0;

  Buffer() = default;
  Buffer(std::uint64_t size) { Allocate(size); }
  Buffer(const void *data, std::uint64_t size)
      : data((std::uint8_t *)data), size(size) {}
  Buffer(const Buffer &) = default;

  static Buffer Copy(Buffer other) {
    Buffer result(other.size);
    std::memcpy(result.data, other.data, other.size);
    return result;
  }

  void Allocate(std::uint64_t size) {
    Release();
    data = (std::uint8_t *)std::malloc(size);
    size = size;
  }

  void Release() {
    std::free(data);
    data = nullptr;
    size = 0;
  }

  template <typename T> T *As() { return (T *)data; }
  operator bool() const { return (bool)data; }
};

} // namespace engine

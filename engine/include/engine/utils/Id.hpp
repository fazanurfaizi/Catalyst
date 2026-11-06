#pragma once

#include <cstdint>
#include <functional>
#include <random>

namespace engine {

struct Id {
public:
  Id() : value_(GenerateRandom()) {};
  Id(std::uint64_t value) : value_(value) {};
  Id(const Id &other) = default;

  operator std::uint64_t() const { return value_; }

private:
  std::uint64_t value_;

  static std::uint64_t GenerateRandom() {
    static std::random_device rnd;
    static std::mt19937 engine(rnd());
    static std::uniform_int_distribution<std::uint64_t> dist;
    return dist(engine);
  }
};

} // namespace engine

namespace std {
template <> struct hash<engine::Id> {
  std::size_t operator()(const engine::Id &id) const {
    return (std::uint64_t)id;
  }
};
} // namespace std

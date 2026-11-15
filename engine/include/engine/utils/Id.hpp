#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <random>

namespace engine {

namespace {

std::uint64_t SplitMix64(std::uint64_t x) {
  x += 0x9e3779b97f4a7c15ull;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebull;
  return x ^ (x >> 31);
}

std::uint64_t RandomSeed() {
  std::random_device rnd;
  const std::uint64_t hi = static_cast<std::uint64_t>(rnd()) << 32;
  const std::uint64_t lo = static_cast<std::uint64_t>(rnd());
  const std::uint64_t clock = static_cast<std::uint64_t>(
      std::chrono::steady_clock::now().time_since_epoch().count());
  return hi ^ lo ^ clock;
}

} // namespace

struct Id {
public:
  Id() : value_(GenerateRandom()) {};
  Id(std::uint64_t value) : value_(value) {};
  Id(const Id &other) = default;

  operator std::uint64_t() const { return value_; }

private:
  std::uint64_t value_;

  static std::uint64_t GenerateRandom() {
    static std::atomic<std::uint64_t> counter{RandomSeed()};

    std::uint64_t v = SplitMix64(
        counter.fetch_add(0x9e3779b97f4a7c15ull, std::memory_order_relaxed));

    if (v == 0)
      v = 1;

    return v;
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

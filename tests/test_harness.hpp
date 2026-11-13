#pragma once

#include <cstdio>

namespace test {

inline int g_failures = 0;
inline int g_checks = 0;

inline void Check(bool cond, const char *expr, const char *file, int line) {
  ++g_checks;
  if (cond)
    return;

  ++g_failures;
  std::printf("FAIL %s:%d: %s\n", file, line, expr);
}

inline int Summary(const char *name) {
  std::printf("[%s] %d checks, %d failures\n", name, g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}

} // namespace test

#define CHECK(cond) ::test::Check((cond), #cond, __FILE__, __LINE__)
#define CHECK_EQ(a, b) CHECK((a) == (b))

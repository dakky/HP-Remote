// Minimales Test-Gerüst für die Host-Tests (g++ auf dem PC, ohne ESP32-Toolchain).
//
//   TEST(name) { REQUIRE(load(...)); CHECK(a == b); CHECK_NEAR(x, 1.0, 0.01); }
//
// REQUIRE bricht den Test bei Fehlschlag ab, CHECK läuft weiter.
//
// Jeder TEST registriert sich selbst, test_main.cpp führt alle aus.
// Rückgabewert des Programms ≠ 0, sobald ein CHECK fehlschlägt.
#pragma once

#include <cmath>
#include <cstdio>

namespace test {

typedef void (*TestFn)();

void registerTest(const char *name, TestFn fn);
void fail(const char *file, int line, const char *msg);

struct Registrar {
  Registrar(const char *name, TestFn fn) { registerTest(name, fn); }
};

}  // namespace test

#define TEST(name)                                         \
  static void test_##name();                               \
  static test::Registrar registrar_##name(#name, test_##name); \
  static void test_##name()

#define CHECK(cond)                                        \
  do {                                                     \
    if (!(cond)) test::fail(__FILE__, __LINE__, #cond);    \
  } while (0)

#define REQUIRE(cond)                                      \
  do {                                                     \
    if (!(cond)) {                                         \
      test::fail(__FILE__, __LINE__, #cond);               \
      return;                                              \
    }                                                      \
  } while (0)

#define CHECK_NEAR(actual, expected, tol)                                   \
  do {                                                                      \
    const double a_ = (actual), e_ = (expected);                            \
    if (!(std::fabs(a_ - e_) <= (tol))) {                                   \
      char msg_[256];                                                       \
      snprintf(msg_, sizeof msg_, "%s = %g, erwartet %g ± %g", #actual, a_, \
               e_, (double)(tol));                                          \
      test::fail(__FILE__, __LINE__, msg_);                                 \
    }                                                                       \
  } while (0)

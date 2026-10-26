// Testlauf: führt alle registrierten TESTs aus und meldet das Ergebnis.
#include "test.h"

#include <vector>

namespace test {

struct Entry {
  const char *name;
  TestFn fn;
};

static std::vector<Entry> &registry() {
  static std::vector<Entry> tests;
  return tests;
}

static int failedChecks = 0;

void registerTest(const char *name, TestFn fn) { registry().push_back({name, fn}); }

void fail(const char *file, int line, const char *msg) {
  failedChecks++;
  printf("  FAIL %s:%d: %s\n", file, line, msg);
}

}  // namespace test

int main() {
  int failedTests = 0;
  for (const test::Entry &t : test::registry()) {
    const int before = test::failedChecks;
    t.fn();
    const bool ok = test::failedChecks == before;
    if (!ok) failedTests++;
    printf("%s %s\n", ok ? "ok  " : "FAIL", t.name);
  }
  printf("\n%d Tests, %d fehlgeschlagen\n", (int)test::registry().size(), failedTests);
  return failedTests == 0 ? 0 : 1;
}

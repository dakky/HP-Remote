// Host-Fake der Log-Ausgabe, siehe fake_log_output.h.
#include "fake_log_output.h"

#include "core/hw/log_output.h"

static std::string collected;

void logOutputWrite(const char *text) { collected += text; }

std::string fakeLogTake() {
  std::string out;
  out.swap(collected);
  return out;
}

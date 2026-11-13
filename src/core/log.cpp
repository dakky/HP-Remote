#include "log.h"

#include "hw/log_output.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>

// Vor dem Laden der Konfiguration nur Fehler, wie zuvor mit der noch leeren
// globalen Konfiguration
static int s_level = 0;

void logSetLevel(int level) { s_level = level; }

int logLevel() { return s_level; }

static const char *prefixFor(int level) {
  switch (level) {
    case 0:  return "[ERR] ";
    case 1:  return "[INF] ";
    case 2:  return "[DBG] ";
    default: return "[ANL] ";
  }
}

void logPrint(int level, const char *fmt, ...) {
  const char *prefix = prefixFor(level);
  const size_t prefixLen = strlen(prefix);

  // Kurze Texte im Stack-Puffer, ohne Heap
  char buf[256];
  memcpy(buf, prefix, prefixLen);
  va_list args;
  va_start(args, fmt);
  const int len = vsnprintf(buf + prefixLen, sizeof buf - prefixLen, fmt, args);
  va_end(args);
  if (len < 0) return;

  if (prefixLen + len < sizeof buf) {
    logOutputWrite(buf);
    return;
  }

  // Langer Text (z. B. JSON-Payloads): zweiter Durchlauf in passender Größe
  std::string text(prefixLen + len + 1, '\0');
  memcpy(&text[0], prefix, prefixLen);
  va_start(args, fmt);
  vsnprintf(&text[prefixLen], len + 1, fmt, args);
  va_end(args);
  text.resize(prefixLen + len);
  logOutputWrite(text.c_str());
}

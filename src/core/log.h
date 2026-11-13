// Logging ohne Hardware: das Modul hält den Log-Level selbst und gibt über
// die Seam core/hw/log_output.h aus.
//
// Level wie cfg.debug_level: 0 = Fehler, 1 = Info, 2 = Debug, 3 = Analyse.
// Ein Makro gibt aus, wenn der eingestellte Level mindestens seinem Level
// entspricht. Die Argumente werden nur dann ausgewertet.
#pragma once

void logSetLevel(int level);
int logLevel();

// Gibt ohne Level-Prüfung aus, mit dem Präfix zu `level` ([ERR], [INF], …).
void logPrint(int level, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

#define LOG_AT_LEVEL_(level, ...) do { if (logLevel() >= (level)) logPrint((level), __VA_ARGS__); } while(0)

#define LOGE(...) LOG_AT_LEVEL_(0, __VA_ARGS__)
#define LOGI(...) LOG_AT_LEVEL_(1, __VA_ARGS__)
#define LOGD(...) LOG_AT_LEVEL_(2, __VA_ARGS__)
#define LOGA(...) LOG_AT_LEVEL_(3, __VA_ARGS__)

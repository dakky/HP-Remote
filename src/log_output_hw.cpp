// Geräte-Adapter der Log-Ausgabe: schreibt auf Serial.
#include <Arduino.h>

#include "core/hw/log_output.h"

void logOutputWrite(const char *text) { Serial.print(text); }

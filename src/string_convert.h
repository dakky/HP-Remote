#pragma once

// Umwandlung zwischen std::string (Logik, z. B. Config-Modell) und Arduino
// String (Webserver, MQTT, WLAN). Nur an den Aufrufstellen außerhalb von
// src/core verwenden, siehe docs/adr/0001-logic-host-buildable.md.

#include <Arduino.h>
#include <string>

inline String toArduino(const std::string &s) { return String(s.c_str(), s.length()); }
inline std::string toStd(const String &s) { return std::string(s.c_str(), s.length()); }

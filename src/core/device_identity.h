// Geräte-Identität: Namen und UID aus den sechs MAC-Bytes, gelesen über die
// Seam core/hw/mac.h.
#pragma once

#include <string>

// Letzte 3 Bytes der MAC als 6-stelliger Hex-Suffix (z. B. "abcdef")
std::string deviceMacSuffix();

// Default-Bezeichner, alle mit MAC-Suffix
std::string defaultDeviceId();      // hp_remote_<suffix>
std::string defaultDeviceName();    // HP-Remote <suffix>
std::string defaultHostname();      // hp-remote-<suffix>
std::string defaultMqttTopicBase(); // heatpump/<device-id>

// Hardware-eindeutige UID: die volle MAC als 12 Hex-Zeichen
// (z. B. "020000abcdef"). Ändert sich nie, auch nicht durch Config-Import
// oder -Export, und braucht kein Dateisystem.
std::string devicePersistentUid();

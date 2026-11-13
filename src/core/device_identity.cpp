#include "device_identity.h"

#include "hw/mac.h"

#include <cstdint>
#include <cstdio>

std::string deviceMacSuffix() {
  uint8_t mac[6];
  macRead(mac);
  char buf[7];
  snprintf(buf, sizeof(buf), "%02x%02x%02x", mac[3], mac[4], mac[5]);
  return buf;
}

std::string devicePersistentUid() {
  uint8_t mac[6];
  macRead(mac);
  char buf[13];
  snprintf(buf, sizeof(buf), "%02x%02x%02x%02x%02x%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return buf;
}

// ─── Default-Bezeichner – alle mit MAC-Suffix, einheitlich "hp_remote" ────────
std::string defaultDeviceId()      { return "hp_remote_" + deviceMacSuffix(); }
std::string defaultDeviceName()    { return "HP-Remote " + deviceMacSuffix(); }
std::string defaultHostname()      { return "hp-remote-" + deviceMacSuffix(); }
std::string defaultMqttTopicBase() { return "heatpump/" + defaultDeviceId(); }

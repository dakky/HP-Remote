// Geräte-Adapter der MAC: liest die WLAN-MAC byteweise (zuverlässig auf ESP32-S3).
#include <esp_mac.h>

#include "core/hw/mac.h"

void macRead(uint8_t mac[6]) { esp_read_mac(mac, ESP_MAC_WIFI_STA); }

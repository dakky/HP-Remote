// Hardware-Seam „MAC“: liefert die sechs Bytes der WLAN-MAC (Station).
// Gerät: mac_hw.cpp (eFuse), Host: tests/fake_mac.cpp (feste Test-MAC).
#pragma once

#include <cstdint>

void macRead(uint8_t mac[6]);

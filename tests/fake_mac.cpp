// Host-Fake der MAC: immer dieselbe Test-MAC 02:00:00:ab:cd:ef.
#include "core/hw/mac.h"

void macRead(uint8_t mac[6]) {
  static const uint8_t testMac[6] = {0x02, 0x00, 0x00, 0xab, 0xcd, 0xef};
  for (int i = 0; i < 6; i++) mac[i] = testMac[i];
}

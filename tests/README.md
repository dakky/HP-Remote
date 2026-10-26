# Host-Tests

Tests, die mit g++ auf dem PC laufen, ohne ESP32-Toolchain. Sie prüfen
Firmware-Module aus reinem C++ (ohne Arduino-Abhängigkeit) mit echten
Snapshots vom Gerät.

```sh
make -C tests          # bauen und ausführen, Rückgabewert ≠ 0 bei Fehlschlag
make -C tests clean    # Build-Ordner entfernen
```

Voraussetzung: `g++` und `make`. Der Build landet in `tests/build/` (ignoriert).

## Aufbau

| Datei                     | Inhalt                                                       |
| ------------------------- | ------------------------------------------------------------ |
| `test.h`, `test_main.cpp` | Minimales Test-Gerüst: `TEST`, `CHECK`, `CHECK_NEAR`, `REQUIRE` |
| `image.h`, `image.cpp`    | `loadGray` (PNG/JPEG → Graustufenpuffer), `distort` (bekannte Verschiebung, Drehung, Maßstab) |
| `*_test.cpp`              | Die Tests; jede Datei wird automatisch mitgebaut             |
| `images/`                 | Testbilder vom Gerät, siehe `images/README.md`               |
| `third_party/`            | [stb_image](https://github.com/nothings/stb) v2.30 (Public Domain), Commit `003501f` |

Zu testende Firmware-Dateien werden im `Makefile` in `FIRMWARE_SRCS`
eingetragen; `src/` liegt im Include-Pfad.

PlatformIO baut nur `src/` und kennt `tests/` nicht (sein Test-Ordner wäre
`test/`), die Host-Tests beeinflussen den Firmware-Build also nicht.

[← zurück zur Übersicht](../README.md) · [Hardware & Montage](HARDWARE.md) · [Inbetriebnahme](SETUP.md) · [Debugging](DEBUG.md) · [MQTT-Beispiele](MQTT-EXAMPLES.md)

# Entwicklung

## Firmware bauen

```sh
pio run -e xiao_esp32s3
```

Oder reproduzierbar im Container (gepinnte Toolchain, wie im Release-Build):

```sh
podman build -t hp-remote-builder -f Containerfile .
podman run --rm -v "$PWD:/workspace:Z" -w /workspace hp-remote-builder run -e xiao_esp32s3
```

Die Artefakte liegen unter `.pio/build/xiao_esp32s3/artifacts/`
(`HP-Remote-OTA.bin` für OTA, `HP-Remote-merged.bin` zum Flashen ab 0x0).

---

## Host-Tests

Reine C++-Module der Firmware (ohne Arduino-Abhängigkeit) werden mit g++ auf
dem PC getestet, mit echten Snapshots vom Gerät. Die ESP32-Toolchain wird
dafür nicht gebraucht, ein Lauf dauert unter einer Sekunde.

**Voraussetzung:** `g++` (C++17) und `make`. Sonst nichts: der Bild-Loader
([stb_image](https://github.com/nothings/stb)) liegt in `tests/third_party/`.

### Tests ausführen

Aus dem Repo-Wurzelverzeichnis:

```sh
make -C tests
```

Das baut alles Nötige nach `tests/build/` und startet die Tests. Ausgabe:

```
ok   loadGray_liest_Snapshot_als_Graustufenpuffer
ok   distort_verschiebt_um_dx_dy
...
8 Tests, 0 fehlgeschlagen
```

Schlägt ein Test fehl, steht darüber die Stelle und die Bedingung:

```
  FAIL image_test.cpp:65: y == 70
FAIL distort_dreht_im_Uhrzeigersinn_um_die_Bildmitte
```

Der Rückgabewert ist dann ≠ 0 (`make` meldet `Error 1`), sodass sich der
Befehl auch in Skripten oder einem Git-Hook verwenden lässt.

### Weitere Befehle

| Befehl                         | Wirkung                                                   |
| ------------------------------ | --------------------------------------------------------- |
| `make -C tests`                | geänderte Dateien neu bauen, alle Tests ausführen         |
| `./tests/build/host_tests`     | die bereits gebaute Test-Binary direkt starten            |
| `make -C tests clean`          | Build-Ordner `tests/build/` löschen                       |
| `make -C tests clean test`     | komplett neu bauen und ausführen                          |
| `gdb tests/build/host_tests`   | im Debugger starten (gebaut mit `-g`)                     |

Die Testbilder werden über einen absoluten Pfad gefunden, die Binary lässt
sich also aus jedem Verzeichnis starten. Wird das Repo verschoben, einmal
`make -C tests clean test` ausführen.

### Aufbau

```
tests/
├── Makefile              ← baut und startet die Tests
├── test.h, test_main.cpp ← Test-Gerüst: TEST, CHECK, CHECK_NEAR, REQUIRE
├── image.h, image.cpp    ← loadGray (PNG/JPEG laden), distort (künstlich verzerren)
├── *_test.cpp            ← die Tests
├── images/               ← Testbilder vom Gerät (siehe images/README.md)
└── third_party/          ← stb_image v2.30 (Public Domain)
```

PlatformIO baut nur `src/` und kennt `tests/` nicht (sein Test-Ordner wäre
`test/`), die Host-Tests beeinflussen den Firmware-Build nicht.

### Einen Test schreiben

Jede Datei `tests/*_test.cpp` wird automatisch mitgebaut, jeder `TEST`
registriert sich selbst:

```cpp
#include "test.h"
#include "image.h"

TEST(beschreibender_name) {
  GrayImage img;
  REQUIRE(loadGray(TEST_IMAGE_DIR "/ref.png", img));   // bricht den Test bei Fehlschlag ab

  Similarity t;          // dx, dy, angle_deg (Uhrzeigersinn), scale
  t.dx = 12;
  const GrayImage moved = distort(img, t);

  CHECK(moved.width == 800);                                // läuft bei Fehlschlag weiter
  CHECK_NEAR(moved.at(412, 300), img.at(400, 300), 2);      // mit Toleranz
}
```

`distort` dreht und skaliert um die Bildmitte und verschiebt danach, mit
demselben Drehsinn wie die Feinrotation (`rotateGrayscaleFb`).

### Firmware-Modul testen

Firmware-Dateien, die getestet werden sollen, in `tests/Makefile` eintragen:

```make
FIRMWARE_SRCS := ../src/frame_align.cpp
```

`src/` liegt im Include-Pfad (`#include "frame_align.h"`). Die Module dürfen
nicht von Arduino, esp32-camera oder ArduinoJson abhängen.

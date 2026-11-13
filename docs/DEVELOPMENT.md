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

Die Logik-Module der Firmware (`src/core/`, siehe
[Logik und Hardware](#logik-und-hardware)) werden mit g++ auf dem PC
getestet, teils mit echten Snapshots vom Gerät. Die ESP32-Toolchain wird
dafür nicht gebraucht, ein Lauf dauert unter einer Sekunde.

**Voraussetzung:** `g++` (C++17) und `make`. Sonst nichts: der Bild-Loader
([stb_image](https://github.com/nothings/stb)) und
[ArduinoJson](https://arduinojson.org) liegen in `tests/third_party/`.

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

Die CI (`.github/workflows/build.yml`) führt die Host-Tests bei jedem Push
im Build-Job vor der Firmware aus; schlagen sie fehl, wird weder Firmware
gebaut noch eine Release angelegt. Ebenso die Tests des Changelog-Skripts,
das beim Release die Release Notes aus `CHANGELOG.md` liest:

```sh
python3 .github/scripts/test_changelog.py
```

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
├── fake_<teil>.*         ← Host-Fakes der Hardware-Seams (z. B. Log-Ausgabe)
├── *_test.cpp            ← die Tests
├── images/               ← Testbilder vom Gerät (siehe images/README.md)
└── third_party/          ← stb_image v2.30 (Public Domain), ArduinoJson 7.4.3 (MIT)
```

`tests/third_party/ArduinoJson.h` ist der Single-Header von ArduinoJson in
derselben Version wie `lib_deps` in `platformio.ini`, damit Host-Tests und
Firmware gleich parsen. Bei einem Versionswechsel beide Stellen gemeinsam
anpassen (Single-Header aus den
[GitHub-Releases](https://github.com/bblanchon/ArduinoJson/releases) als
`ArduinoJson.h` ablegen).

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

### Logik-Modul testen

`tests/Makefile` baut alle `.cpp`-Dateien unter `src/core/` (auch in
Unterordnern) automatisch mit und übersetzt zusätzlich jeden Header dort
einzeln, eine Dateiliste gibt es nicht. `src/` liegt im
Include-Pfad, Tests binden Logik-Module als `#include "core/frame_align.h"` ein.

---

## Logik und Hardware

Die Regel steht in [ADR 0001](adr/0001-logic-host-buildable.md). Kurz:

```
src/
├── core/                 ← Logik: reines C++, baut auf Gerät und Host
│   ├── frame_align.*
│   ├── app_config.*      ← Konfiguration: Defaults, Config ↔ JSON
│   ├── log.*             ← Log-Makros, Level per logSetLevel()
│   ├── device_identity.* ← Geräte-Namen und UID aus der MAC
│   └── hw/<teil>.h       ← Hardware-Seams: nur Deklarationen freier Funktionen
├── <teil>_hw.cpp         ← Geräte-Adapter: implementiert core/hw/<teil>.h
└── …                     ← übrige Firmware (Webinterface, MQTT, WLAN, …)
tests/
└── fake_<teil>.cpp       ← Host-Fake: implementiert core/hw/<teil>.h für Tests
```

- **Logik-Verzeichnis `src/core/`:** Hier liegt alles, was Entscheidungen
  trifft oder rechnet. PlatformIO baut es automatisch mit, `make -C tests`
  ebenfalls.
- **Header-Regel:** Dateien unter `src/core/` binden weder `Arduino.h` noch
  `esp_*`-Header noch Libraries mit Hardwarebezug ein (WebServer, PubSubClient,
  Wire, WiFi, esp32-camera). Erlaubt ist ArduinoJson (header-only). Texte sind
  `std::string`, nicht Arduino `String`; umgewandelt wird erst außerhalb von
  `src/core/`. Ein Verstoß lässt den Host-Build scheitern und damit CI und
  Release.
- **Hardware-Seams:** Braucht die Logik Hardware (MAC-Adresse, Log-Ausgabe, …),
  deklariert sie dafür freie Funktionen in einem kleinen Header pro
  Hardware-Teil unter `src/core/hw/`. Keine Klassen mit `virtual`, keine
  Übergabe zur Laufzeit: welche Implementierung gilt, entscheidet der Build.
  Daten wie Config, Bilder oder Messwerte kommen als Parameter, nicht über
  eine Seam.
- **Geräte-Adapter:** Die Implementierung für das Gerät liegt in `src/`
  außerhalb von `core/` und spricht nur die Hardware an, ohne eigene Logik.
- **Host-Fakes:** Die Implementierung für den Host liegt in `tests/` und
  liefert feste Werte oder sammelt Aufrufe, damit Tests sie prüfen können.

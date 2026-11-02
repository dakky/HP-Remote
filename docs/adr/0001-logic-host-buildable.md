# Logik ist host-baubar, Hardware sitzt hinter einfachen Adaptern

Die Firmware-Logik soll ohne Gerät mit g++ testbar sein. Deshalb liegen Logik-Module in `src/core/` und binden weder `Arduino.h` noch `esp_*` noch Libraries mit Hardwarebezug (WebServer, PubSubClient, Wire, WiFi) ein. ArduinoJson ist erlaubt, da header-only und auf dem Host lauffähig. Logik-Module verwenden `std::string` statt Arduino `String`; umgewandelt wird erst in den Adaptern.

Hardware erreicht die Logik ausschließlich über freie Funktionen, deklariert in `src/core/hw/<teil>.h` (Link-Seam). Den Geräte-Adapter dazu gibt es in `src/`, den Host-Fake in `tests/`. Adapter enthalten keine Logik. Daten (Config, Bilder, Messwerte) kommen als Parameter. `tests/Makefile` kompiliert alles unter `src/core/`, so dass ein Verstoß den Host-Build und damit CI und Release scheitern lässt.

## Considered Options

- **Arduino-`String`-Nachbau für den Host:** weniger Umbau, holt aber Arduino zurück in die Logik.
- **Interface-Klassen mit `virtual`, zur Laufzeit übergeben:** erst sinnvoll, wenn zwei Implementierungen gleichzeitig existieren müssen, was auf dem Gerät nicht vorkommt. Kostet Verdrahtung an jeder Aufrufstelle.
- **Flaches `src/` mit Liste im Makefile:** neue Logik-Dateien würden ungeprüft bleiben, bis jemand die Liste pflegt.

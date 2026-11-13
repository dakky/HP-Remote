# Changelog

Diese Datei listet nutzerrelevante Änderungen am HP-Remote, von Hand geschrieben (kein Auto-Generator). Neueste Einträge oben. Neues kommt unter `## Unveröffentlicht`; beim Release benennt der Workflow den Abschnitt in `## <Version> – <Datum>` um und übernimmt seinen Text als Release Notes. Details siehe [Dokumentation](docs/SETUP.md).

## Unveröffentlicht
- Interner Umbau, Verhalten unverändert.

## 1.0.0 – 2026-11-02
- Erste stabile Version als eigenständiges Projekt, mit Releases und Web Installer aus diesem Repository.
- Neu: **Auto Align** richtet jedes Bild an den Ankerlinien des Displays aus, sodass die ROIs nach einer leichten Kameraverschiebung weiter passen. Standardmäßig aus, siehe [SETUP.md](docs/SETUP.md#auto-align-optional).
- Test Detection wertet jetzt dasselbe Bild aus wie der reguläre Loop (inklusive Feinrotation).
- Beispiel-Konfiguration `docs/HP-Remote-config.json` aktualisiert.

## 2026-07-02
- Heizraum-Sensor wird jetzt unabhängig von MQTT kontinuierlich gelesen (Übertragung bleibt an aktives MQTT gekoppelt) – siehe [SETUP.md](docs/SETUP.md#heizraum-sensor-optional-sht31).
- I2C-Bus-Scan (`/api/i2c/scan`) listet jetzt alle gefundenen Geräte (inkl. SHT31), nicht nur den PCF8574 – neuer Button "Scan I2C bus" im Webinterface, siehe [DEBUG.md](docs/DEBUG.md).

## 2026-06-28
- Optionaler Heizraum-Sensor (Temperatur/Luftfeuchte, SHT31) hinzugefügt – siehe [SETUP.md](docs/SETUP.md#heizraum-sensor-optional-sht31).

# Testbilder für Auto Align

Echte Snapshots vom Gerät (WPT-Display) für die Host-Tests der Linienerkennung
und Ausrichtung.

Aufnahme über `/api/snapshot` (Rohbild, ohne Feinrotation):
800×600, Graustufen, `jpeg_quality` 85, nur Auto White Balance an,
Exposure 200, Gain 5. Abgelegt als verlustfreies 8-bit-Graustufen-PNG.

| Datei               | Kamera                            | Ziffernerkennung          |
| ------------------- | --------------------------------- | ------------------------- |
| `ref.png`           | Referenzlage, ROIs passen         | ok                        |
| `shift_up.png`      | nach oben verschoben              | fehlgeschlagen            |
| `shift_up_side.png` | nach oben und seitlich verschoben | fehlgeschlagen            |
| `shift_side.png`    | nur seitlich verschoben           | fehlgeschlagen            |
| `shift_side_rot.png` | nach Wiedereinbau: ca. 10 px seitlich, ca. −0,8° gedreht | teilweise (Set Temp ok, Water Temp/Uhrzeit nicht) |

Drehung, Maßstab und ein Bild ohne Ankerlinien werden in den Tests synthetisch
aus `ref.png` erzeugt.

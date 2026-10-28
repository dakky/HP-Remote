// Frame Align: Lage des WPT-Displays im Kamerabild über drei feste Ankerlinien
// bestimmen (zwei waagerechte Linien über fast die ganze Breite, eine senkrechte
// Trennlinie zwischen "Set Temp" und "Water Temp").
//
// Reines C++ ohne Arduino-, esp32-camera- oder ArduinoJson-Abhängigkeit, damit
// das Modul auch auf dem PC getestet werden kann (tests/frame_align_test.cpp).
#pragma once

#include <cstdint>

struct AnchorPoint {
  float x;
  float y;
};

// Gemessene Ankerlinien in Pixelkoordinaten des Rohbilds.
// Die waagerechten Linien werden in je einem schmalen Streifen links und rechts
// gemessen (zwei Punkte je Linie), die senkrechte Linie auf halber Höhe
// zwischen den waagerechten Linien.
struct AnchorLines {
  AnchorPoint top_left;
  AnchorPoint top_right;
  AnchorPoint bottom_left;
  AnchorPoint bottom_right;
  AnchorPoint vertical;
};

enum class AnchorError {
  None,
  InvalidImage,   // kein Bild oder zu klein
  TopLine,        // obere waagerechte Linie nicht gefunden
  BottomLine,     // untere waagerechte Linie nicht gefunden
  VerticalLine,   // senkrechte Trennlinie nicht gefunden
  Inconsistent,   // Linien gefunden, aber Lage passt nicht zusammen
};

// Misst die drei Ankerlinien in einem Graustufenbild (zeilenweise, 1 Byte/Pixel).
// Liefert AnchorError::None und füllt out, sonst bleibt out unverändert.
AnchorError measureAnchorLines(const uint8_t *gray, int W, int H, AnchorLines &out);

// Kurzer englischer Text für API-Fehlermeldungen.
const char *anchorErrorText(AnchorError err);

// Frame Align: Lage des WPT-Displays im Kamerabild über drei feste Ankerlinien
// bestimmen (zwei waagerechte Linien über fast die ganze Breite, eine senkrechte
// Trennlinie zwischen "Set Temp" und "Water Temp").
//
// Logik-Modul (ADR 0001): reines C++ ohne Arduino- oder esp32-camera-Abhängigkeit,
// damit es auch auf dem PC getestet werden kann (tests/frame_align_test.cpp).
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

// ── Ausrichten ────────────────────────────────────────────────────────────

// Ähnlichkeitstransformation von der Referenzlage in das aktuelle Bild:
// Ein Punkt p der Referenz liegt im Bild bei  c + scale · R(angle_deg) · (p − c) + (dx, dy),
// c = Bildmitte. Positiver Winkel = im Uhrzeigersinn (wie die Feinrotation).
struct AlignTransform {
  float dx;
  float dy;
  float angle_deg;
  float scale;
};

static constexpr AlignTransform kAlignIdentity = {0.0f, 0.0f, 0.0f, 1.0f};

enum class AlignStatus {
  Ok,            // Linien gemessen, Transformation plausibel
  Fallback,      // Linien fehlen oder Ergebnis unplausibel: letzte gültige Transformation
  NoReference,   // keine Referenz gesetzt: Bild bleibt unverändert
};

struct AlignResult {
  AlignStatus status;
  AlignTransform transform;   // anzuwendende Transformation
  AnchorError error;          // Fehler der Linienmessung (None, wenn gemessen)
  bool lines_found;           // lines gültig (auch bei unplausiblem Ergebnis)
  AnchorLines lines;          // gemessene Ankerlinien im Rohbild
};

// Misst die Ankerlinien und bestimmt die Transformation gegenüber ref.
// ref == nullptr → NoReference mit Identität. Linien fehlen oder Ergebnis
// außerhalb der Plausibilitätsgrenzen → Fallback mit lastValid.
AlignResult estimateAlignment(const uint8_t *gray, int W, int H, const AnchorLines *ref,
                              const AlignTransform &lastValid);

// Bringt das Bild in-place auf die Referenzlage (Nearest Neighbor).
// scratch: Hilfspuffer mit W·H Bytes. Pixel außerhalb des Bilds werden
// schwarz (dunkler Display-Hintergrund). Identität → keine Änderung.
void applyAlignment(uint8_t *gray, int W, int H, const AlignTransform &t, uint8_t *scratch);

const char *alignStatusText(AlignStatus status);

// Grund für Fallback als kurzer englischer Text, sonst nullptr.
const char *alignFallbackText(const AlignResult &r);

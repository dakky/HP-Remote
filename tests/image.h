// Bild-Hilfen für die Host-Tests: Testbilder laden und synthetisch verzerren.
#pragma once

#include <cstdint>
#include <vector>

// 8-bit-Graustufenbild, zeilenweise (wie der Kamera-Framebuffer).
struct GrayImage {
  int width = 0;
  int height = 0;
  std::vector<uint8_t> pixels;

  uint8_t &at(int x, int y) { return pixels[y * width + x]; }
  uint8_t at(int x, int y) const { return pixels[y * width + x]; }
};

// Lädt ein PNG oder JPEG als Graustufenbild. false, wenn die Datei fehlt
// oder nicht lesbar ist.
bool loadGray(const char *path, GrayImage &out);

// Ähnlichkeitstransformation, wie sie Auto Align ausgleichen soll.
// Ein Punkt p im Quellbild landet bei  c + scale · R(angle_deg) · (p − c) + (dx, dy),
// c = Bildmitte (W/2, H/2). Drehsinn wie die Feinrotation (rotateGrayscaleFb):
// positiver Winkel = im Uhrzeigersinn auf dem Bildschirm (y zeigt nach unten).
struct Similarity {
  double dx = 0;
  double dy = 0;
  double angle_deg = 0;
  double scale = 1;
};

// Erzeugt ein künstlich verzerrtes Bild (bilinear abgetastet), z. B. als
// "verrutschte Kamera" aus ref.png. Pixel, die außerhalb des Quellbilds
// liegen, werden schwarz (0): Der Display-Hintergrund ist dunkel, ein grauer
// Rand (wie bei rotateGrayscaleFb) würde eine falsche Kante erzeugen.
GrayImage distort(const GrayImage &src, const Similarity &t);

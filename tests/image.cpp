#include "image.h"

#include <cmath>
#include <cstring>

#include "stb_image.h"

bool loadGray(const char *path, GrayImage &out) {
  int w = 0, h = 0, channels = 0;
  // 1 = auf einen Kanal (Graustufen) umrechnen, egal wie die Datei gespeichert ist
  uint8_t *data = stbi_load(path, &w, &h, &channels, 1);
  if (!data) return false;

  out.width = w;
  out.height = h;
  out.pixels.resize((size_t)w * h);
  memcpy(out.pixels.data(), data, out.pixels.size());
  stbi_image_free(data);
  return true;
}

GrayImage distort(const GrayImage &src, const Similarity &t) {
  GrayImage out;
  out.width = src.width;
  out.height = src.height;
  out.pixels.assign(src.pixels.size(), 0);

  const double rad = t.angle_deg * M_PI / 180.0;
  const double cs = cos(rad);
  const double sn = sin(rad);
  const double cx = src.width / 2.0;
  const double cy = src.height / 2.0;

  for (int y = 0; y < out.height; y++) {
    for (int x = 0; x < out.width; x++) {
      // Rückwärts: Quellposition für Zielpixel (x,y), p = c + R(−a) · (q − c − d) / s
      const double qx = (x - cx - t.dx) / t.scale;
      const double qy = (y - cy - t.dy) / t.scale;
      const double sx = cx + qx * cs + qy * sn;
      const double sy = cy - qx * sn + qy * cs;

      const int x0 = (int)floor(sx);
      const int y0 = (int)floor(sy);
      const double fx = sx - x0;
      const double fy = sy - y0;
      double v = 0;
      for (int j = 0; j < 2; j++) {
        for (int i = 0; i < 2; i++) {
          const int px = x0 + i;
          const int py = y0 + j;
          if (px < 0 || px >= src.width || py < 0 || py >= src.height) continue;
          v += src.at(px, py) * (i ? fx : 1 - fx) * (j ? fy : 1 - fy);
        }
      }
      out.at(x, y) = (uint8_t)lround(v);
    }
  }
  return out;
}

// Tests der Bild-Hilfsfunktionen des Gerüsts (Laden, synthetisch verzerren).
#include "test.h"
#include "image.h"

// Erwartete Werte unabhängig ermittelt mit ImageMagick:
//   magick ref.png -format "%[fx:round(255*p{x,y})]" info:
TEST(loadGray_liest_Snapshot_als_Graustufenpuffer) {
  GrayImage img;
  REQUIRE(loadGray(TEST_IMAGE_DIR "/ref.png", img));
  REQUIRE(img.width == 800);
  REQUIRE(img.height == 600);
  REQUIRE(img.pixels.size() == 800u * 600u);
  CHECK(img.at(0, 0) == 2);
  CHECK(img.at(400, 300) == 255);
  CHECK(img.at(123, 456) == 46);
  CHECK(img.at(799, 599) == 0);
}

// Schwarzes Bild mit einem einzelnen hellen Punkt
static GrayImage pointImage(int w, int h, int px, int py) {
  GrayImage img;
  img.width = w;
  img.height = h;
  img.pixels.assign((size_t)w * h, 0);
  img.at(px, py) = 255;
  return img;
}

static void brightest(const GrayImage &img, int &bx, int &by) {
  bx = by = -1;
  int best = -1;
  for (int y = 0; y < img.height; y++)
    for (int x = 0; x < img.width; x++)
      if (img.at(x, y) > best) {
        best = img.at(x, y);
        bx = x;
        by = y;
      }
}

TEST(distort_verschiebt_um_dx_dy) {
  const GrayImage src = pointImage(100, 80, 30, 20);
  Similarity t;
  t.dx = 5;
  t.dy = -3;
  const GrayImage out = distort(src, t);
  REQUIRE(out.width == 100 && out.height == 80);
  int x, y;
  brightest(out, x, y);
  CHECK(x == 35);
  CHECK(y == 17);
  CHECK(out.at(35, 17) == 255);
}

// Mitte c = (50,50). Punkt rechts der Mitte (70,50) wandert bei +90° im
// Uhrzeigersinn nach unten: c + R(90°)·(20,0) = (50,70).
TEST(distort_dreht_im_Uhrzeigersinn_um_die_Bildmitte) {
  const GrayImage src = pointImage(100, 100, 70, 50);
  Similarity t;
  t.angle_deg = 90;
  const GrayImage out = distort(src, t);
  int x, y;
  brightest(out, x, y);
  CHECK(x == 50);
  CHECK(y == 70);
}

// c + 2·((60,50) − c) = (70,50)
TEST(distort_skaliert_um_die_Bildmitte) {
  const GrayImage src = pointImage(100, 100, 60, 50);
  Similarity t;
  t.scale = 2;
  const GrayImage out = distort(src, t);
  int x, y;
  brightest(out, x, y);
  CHECK(x == 70);
  CHECK(y == 50);
}

// Erst drehen und skalieren um die Mitte, dann verschieben:
// (70,50) → R(90°)·(20,0) = (0,20) → ·1,5 = (0,30) → + c + (3,4) = (53,84)
TEST(distort_kombiniert_Drehung_Massstab_und_Verschiebung) {
  const GrayImage src = pointImage(100, 100, 70, 50);
  Similarity t;
  t.dx = 3;
  t.dy = 4;
  t.angle_deg = 90;
  t.scale = 1.5;
  const GrayImage out = distort(src, t);
  int x, y;
  brightest(out, x, y);
  CHECK(x == 53);
  CHECK(y == 84);
}

TEST(distort_fuellt_Rand_ausserhalb_des_Quellbilds_schwarz) {
  GrayImage src;
  src.width = 20;
  src.height = 10;
  src.pixels.assign(200, 200);
  Similarity t;
  t.dx = 5;
  const GrayImage out = distort(src, t);
  CHECK(out.at(0, 5) == 0);
  CHECK(out.at(4, 5) == 0);
  CHECK(out.at(5, 5) == 200);
  CHECK(out.at(19, 5) == 200);
}

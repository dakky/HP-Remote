// Tests des Frame-Align-Moduls: Ankerlinien messen.
#include "test.h"
#include "image.h"

#include "frame_align.h"

#include <string>

static AnchorError measure(const GrayImage &img, AnchorLines &lines) {
  return measureAnchorLines(img.pixels.data(), img.width, img.height, lines);
}

// Erwartete Lagen unabhängig mit ImageMagick abgelesen (helle Pixel in einer
// Spalte bzw. Zeile von ref.png):
//   obere Linie   x=176: y 218–221 hell → Mitte ≈ 219,5;  x=624: Mitte ≈ 220
//   untere Linie  x=176: y 465–467 hell → Mitte ≈ 466;    x=624: Mitte ≈ 465
//   senkrechte Linie y=350: x 400–403 hell → Mitte ≈ 402
TEST(measureAnchorLines_findet_alle_drei_Linien_im_Referenzbild) {
  GrayImage img;
  REQUIRE(loadGray(TEST_IMAGE_DIR "/ref.png", img));

  AnchorLines l;
  REQUIRE(measure(img, l) == AnchorError::None);

  CHECK_NEAR(l.top_left.y, 219.5, 1.5);
  CHECK_NEAR(l.top_right.y, 220, 1.5);
  CHECK_NEAR(l.bottom_left.y, 466, 1.5);
  CHECK_NEAR(l.bottom_right.y, 465, 1.5);
  CHECK_NEAR(l.vertical.x, 402, 1.5);

  // Messpunkte liegen links bzw. rechts der Bildmitte, die senkrechte Linie
  // zwischen den waagerechten
  CHECK(l.top_left.x < 400 && l.top_right.x > 400);
  CHECK(l.bottom_left.x < 400 && l.bottom_right.x > 400);
  CHECK(l.vertical.y > 230 && l.vertical.y < 455);
}

// Bild ohne Ankerlinien, synthetisch: Kamera teilweise abgedeckt (untere
// Bildhälfte schwarz) → die untere Linie fehlt.
TEST(measureAnchorLines_meldet_Fehler_bei_abgedeckter_unterer_Haelfte) {
  GrayImage img;
  REQUIRE(loadGray(TEST_IMAGE_DIR "/ref.png", img));
  for (int y = img.height / 2; y < img.height; y++)
    for (int x = 0; x < img.width; x++) img.at(x, y) = 0;

  AnchorLines l = {};
  CHECK(measure(img, l) == AnchorError::BottomLine);
  CHECK(l.vertical.x == 0);   // out bleibt unverändert
}

TEST(measureAnchorLines_meldet_Fehler_bei_schwarzem_Bild) {
  GrayImage img;
  img.width = 800;
  img.height = 600;
  img.pixels.assign(800u * 600u, 0);
  AnchorLines l;
  CHECK(measure(img, l) == AnchorError::TopLine);
}

// Display aus, nur Sensorrauschen: keine Linie darf gefunden werden.
TEST(measureAnchorLines_meldet_Fehler_bei_verrauschtem_Bild) {
  GrayImage img;
  img.width = 800;
  img.height = 600;
  img.pixels.resize(800u * 600u);
  uint32_t seed = 12345;
  for (uint8_t &p : img.pixels) {
    seed = seed * 1664525u + 1013904223u;   // LCG, reproduzierbar
    p = (uint8_t)(seed >> 24);
  }
  AnchorLines l;
  CHECK(measure(img, l) != AnchorError::None);
}

// Ohne die senkrechte Trennlinie (übermalt mit Hintergrund) wird sie als
// fehlend gemeldet, obwohl beide waagerechten Linien da sind.
TEST(measureAnchorLines_meldet_fehlende_senkrechte_Linie) {
  GrayImage img;
  REQUIRE(loadGray(TEST_IMAGE_DIR "/ref.png", img));
  for (int y = 240; y < 450; y++)
    for (int x = 390; x < 415; x++) img.at(x, y) = 10;
  AnchorLines l;
  CHECK(measure(img, l) == AnchorError::VerticalLine);
}

// ── Künstlich verzerrte Referenz: Die gemessenen Linien müssen der bekannten
// Transformation aus distort() folgen.

static GrayImage loadRef() {
  GrayImage img;
  loadGray(TEST_IMAGE_DIR "/ref.png", img);
  return img;
}

static double slope(const AnchorPoint &a, const AnchorPoint &b) { return (b.y - a.y) / (b.x - a.x); }

// Lage einer Linie (durch p mit Steigung s) an der Stelle x
static double yAt(const AnchorPoint &p, double s, double x) { return p.y + s * (x - p.x); }

TEST(measureAnchorLines_folgt_einer_Verschiebung) {
  const GrayImage ref = loadRef();
  REQUIRE(ref.width == 800);
  AnchorLines r;
  REQUIRE(measure(ref, r) == AnchorError::None);

  Similarity t;
  t.dx = 25;
  t.dy = -30;
  AnchorLines m;
  REQUIRE(measure(distort(ref, t), m) == AnchorError::None);

  // Streifen bleiben an derselben x-Stelle, die Linien sind dort waagerecht
  CHECK_NEAR(m.top_left.y, r.top_left.y - 30, 1);
  CHECK_NEAR(m.top_right.y, r.top_right.y - 30, 1);
  CHECK_NEAR(m.bottom_left.y, r.bottom_left.y - 30, 1);
  CHECK_NEAR(m.bottom_right.y, r.bottom_right.y - 30, 1);
  CHECK_NEAR(m.vertical.x, r.vertical.x + 25, 1);
}

TEST(measureAnchorLines_folgt_einer_Drehung) {
  const GrayImage ref = loadRef();
  AnchorLines r;
  REQUIRE(measure(ref, r) == AnchorError::None);

  for (double deg : {-4.0, -1.5, 2.5, 4.5}) {
    Similarity t;
    t.angle_deg = deg;
    AnchorLines m;
    REQUIRE(measure(distort(ref, t), m) == AnchorError::None);

    // Drehung im Uhrzeigersinn → Linien steigen nach rechts um tan(Winkel)
    const double ds = tan(deg * M_PI / 180);
    CHECK_NEAR(slope(m.top_left, m.top_right), slope(r.top_left, r.top_right) + ds, 0.005);
    CHECK_NEAR(slope(m.bottom_left, m.bottom_right), slope(r.bottom_left, r.bottom_right) + ds, 0.005);

    // Senkrechte Linie: Referenzpunkt um die Bildmitte drehen, dann entlang der
    // gedrehten Linie auf die gemessene Höhe gehen (x sinkt um tan·Δy)
    const double a = deg * M_PI / 180, cx = 400, cy = 300;
    const double px = cx + cos(a) * (r.vertical.x - cx) - sin(a) * (r.vertical.y - cy);
    const double py = cy + sin(a) * (r.vertical.x - cx) + cos(a) * (r.vertical.y - cy);
    CHECK_NEAR(m.vertical.x, px - (m.vertical.y - py) * ds, 1);
  }
}

TEST(measureAnchorLines_folgt_einem_Massstab) {
  const GrayImage ref = loadRef();
  AnchorLines r;
  REQUIRE(measure(ref, r) == AnchorError::None);

  for (double s : {0.92, 1.08}) {
    Similarity t;
    t.scale = s;
    AnchorLines m;
    REQUIRE(measure(distort(ref, t), m) == AnchorError::None);

    // Abstand der waagerechten Linien wächst mit dem Maßstab
    const double gapRef = r.bottom_left.y - yAt(r.top_left, slope(r.top_left, r.top_right), r.bottom_left.x);
    const double gap = m.bottom_left.y - yAt(m.top_left, slope(m.top_left, m.top_right), m.bottom_left.x);
    CHECK_NEAR(gap, gapRef * s, 1.5);
    CHECK_NEAR(m.vertical.x, 400 + (r.vertical.x - 400) * s, 1);
  }
}

TEST(measureAnchorLines_folgt_kombinierter_Verzerrung_an_den_Grenzen) {
  const GrayImage ref = loadRef();
  AnchorLines r;
  REQUIRE(measure(ref, r) == AnchorError::None);

  // ±40 px bei 640×480 entsprechen ±50 px bei 800×600
  Similarity t;
  t.dx = -50;
  t.dy = 50;
  t.angle_deg = -5;
  t.scale = 0.9;
  AnchorLines m;
  REQUIRE(measure(distort(ref, t), m) == AnchorError::None);
  CHECK_NEAR(slope(m.top_left, m.top_right), slope(r.top_left, r.top_right) + tan(-5 * M_PI / 180), 0.005);
}

// ── Echte Snapshots mit verrutschter Kamera (tests/images/README.md)
// Erwartete Lagen unabhängig mit ImageMagick abgelesen: Mitte der hellen Pixel
// in den Spalten x=176 / x=624 bzw. in der Zeile y=340.
TEST(measureAnchorLines_findet_Linien_in_echten_verschobenen_Snapshots) {
  struct Case {
    const char *file;
    double topL, topR, bottomL, bottomR, vertX;
  };
  const Case cases[] = {
      {"/shift_up.png", 216, 212.5, 462.5, 457.5, 403},
      {"/shift_up_side.png", 212, 213, 458.5, 456.5, 380},
      {"/shift_side.png", 219.5, 218.5, 465.5, 463, 378},
      {"/shift_side_rot.png", 221.5, 214.5, 468, 459, 412.5},
  };
  for (const Case &c : cases) {
    GrayImage img;
    REQUIRE(loadGray((std::string(TEST_IMAGE_DIR) + c.file).c_str(), img));
    AnchorLines m;
    const AnchorError err = measure(img, m);
    if (err != AnchorError::None) printf("  %s: %s\n", c.file, anchorErrorText(err));
    REQUIRE(err == AnchorError::None);

    // Streifen liegen bei 800 px Breite um x=176 und x=624
    CHECK_NEAR(m.top_left.y, c.topL, 1.5);
    CHECK_NEAR(m.top_right.y, c.topR, 1.5);
    CHECK_NEAR(m.bottom_left.y, c.bottomL, 1.5);
    CHECK_NEAR(m.bottom_right.y, c.bottomR, 1.5);
    CHECK_NEAR(m.vertical.x, c.vertX, 1.5);
  }
}

TEST(measureAnchorLines_meldet_ungueltiges_Bild) {
  AnchorLines l;
  CHECK(measureAnchorLines(nullptr, 800, 600, l) == AnchorError::InvalidImage);
  const uint8_t tiny[16] = {};
  CHECK(measureAnchorLines(tiny, 4, 4, l) == AnchorError::InvalidImage);
}

// ── Ausrichten (estimateAlignment + applyAlignment)

// Erkennen und im Puffer ausrichten, wie cameraPrepareFrame es tut
static AlignResult align(GrayImage &img, const AnchorLines *ref, const AlignTransform &lastValid) {
  const AlignResult r = estimateAlignment(img.pixels.data(), img.width, img.height, ref, lastValid);
  std::vector<uint8_t> scratch(img.pixels.size());
  applyAlignment(img.pixels.data(), img.width, img.height, r.transform, scratch.data());
  return r;
}

TEST(align_ohne_Referenz_laesst_Bild_unveraendert) {
  GrayImage img = distort(loadRef(), Similarity{20, -15, 2, 1});
  const std::vector<uint8_t> before = img.pixels;

  const AlignResult r = align(img, nullptr, kAlignIdentity);

  CHECK(r.status == AlignStatus::NoReference);
  CHECK(r.transform.dx == 0 && r.transform.dy == 0 && r.transform.angle_deg == 0 && r.transform.scale == 1);
  CHECK(img.pixels == before);
}

// Bekannte Verzerrung der Referenz (distort benutzt dieselbe Konvention wie
// AlignTransform: Referenz → Bild) wird wiedergefunden.
TEST(estimateAlignment_findet_kuenstliche_Verzerrung_wieder) {
  const GrayImage ref = loadRef();
  AnchorLines r;
  REQUIRE(measure(ref, r) == AnchorError::None);

  const Similarity cases[] = {
      {0, 0, 0, 1},
      {25, -30, 0, 1},
      {0, 0, 3, 1},
      {0, 0, 0, 1.07},
      {-12, 18, -2.5, 0.94},
      {40, 35, 4, 1.05},
  };
  for (const Similarity &t : cases) {
    const GrayImage img = distort(ref, t);
    const AlignResult a = estimateAlignment(img.pixels.data(), img.width, img.height, &r, kAlignIdentity);
    REQUIRE(a.status == AlignStatus::Ok);
    CHECK(a.lines_found);
    CHECK_NEAR(a.transform.dx, t.dx, 1.0);
    CHECK_NEAR(a.transform.dy, t.dy, 1.0);
    CHECK_NEAR(a.transform.angle_deg, t.angle_deg, 0.15);
    CHECK_NEAR(a.transform.scale, t.scale, 0.01);
  }
}

// Echte Snapshots mit verrutschter Kamera: Nach dem Ausrichten liegen die
// Ankerlinien wieder an der Referenzlage (gemessen auf dem ausgerichteten Bild).
TEST(align_bringt_echte_verschobene_Snapshots_auf_die_Referenzlage) {
  const GrayImage refImg = loadRef();
  AnchorLines r;
  REQUIRE(measure(refImg, r) == AnchorError::None);

  for (const char *file : {"/shift_up.png", "/shift_up_side.png", "/shift_side.png", "/shift_side_rot.png"}) {
    GrayImage img;
    REQUIRE(loadGray((std::string(TEST_IMAGE_DIR) + file).c_str(), img));
    const AlignResult a = align(img, &r, kAlignIdentity);
    if (a.status != AlignStatus::Ok) printf("  %s: %s\n", file, anchorErrorText(a.error));
    REQUIRE(a.status == AlignStatus::Ok);

    AnchorLines m;
    REQUIRE(measure(img, m) == AnchorError::None);
    CHECK_NEAR(m.top_left.y, r.top_left.y, 2);
    CHECK_NEAR(m.top_right.y, r.top_right.y, 2);
    CHECK_NEAR(m.bottom_left.y, r.bottom_left.y, 2);
    CHECK_NEAR(m.bottom_right.y, r.bottom_right.y, 2);
    CHECK_NEAR(m.vertical.x, r.vertical.x, 2);
  }
}

// Leeres oder verrauschtes Bild: keine Linien → letzte gültige Transformation
TEST(estimateAlignment_faellt_bei_leerem_oder_verrauschtem_Bild_zurueck) {
  AnchorLines r;
  REQUIRE(measure(loadRef(), r) == AnchorError::None);

  GrayImage black;
  black.width = 800;
  black.height = 600;
  black.pixels.assign(800u * 600u, 0);

  GrayImage noise = black;
  uint32_t seed = 4711;
  for (uint8_t &p : noise.pixels) {
    seed = seed * 1664525u + 1013904223u;
    p = (uint8_t)(seed >> 24);
  }

  const AlignTransform last = {12, -7, 1.5f, 1.03f};
  for (const GrayImage *img : {&black, &noise}) {
    const AlignResult a = estimateAlignment(img->pixels.data(), 800, 600, &r, last);
    CHECK(a.status == AlignStatus::Fallback);
    CHECK(!a.lines_found);
    CHECK(a.error != AnchorError::None);
    CHECK(a.transform.dx == 12 && a.transform.dy == -7 && a.transform.angle_deg == 1.5f &&
          a.transform.scale == 1.03f);

    // Ohne bisherige gültige Transformation: Identität
    const AlignResult b = estimateAlignment(img->pixels.data(), 800, 600, &r, kAlignIdentity);
    CHECK(b.status == AlignStatus::Fallback);
    CHECK(b.transform.dx == 0 && b.transform.dy == 0 && b.transform.angle_deg == 0 && b.transform.scale == 1);
  }
}

// Linien gefunden, aber Transformation außerhalb der Plausibilitätsgrenzen
// (Verschiebung ±40 px bei 640×480 = ±50 px bei 800×600, Drehung ±5°,
// Maßstab 0,9–1,1) → Fehlerkennung, letzte gültige Transformation
TEST(estimateAlignment_verwirft_unplausible_Transformation) {
  const GrayImage ref = loadRef();
  AnchorLines r;
  REQUIRE(measure(ref, r) == AnchorError::None);

  const Similarity cases[] = {
      {0, 65, 0, 1},
      {-65, 0, 0, 1},
      {0, 0, 5.6, 1},
      {0, 0, 0, 1.14},
      {0, 0, 0, 0.86},
  };
  const AlignTransform last = {3, 4, -0.5f, 1.01f};
  for (const Similarity &t : cases) {
    const GrayImage img = distort(ref, t);
    const AlignResult a = estimateAlignment(img.pixels.data(), img.width, img.height, &r, last);
    if (!a.lines_found) printf("  dx=%g dy=%g a=%g s=%g: %s\n", t.dx, t.dy, t.angle_deg, t.scale, anchorErrorText(a.error));
    REQUIRE(a.lines_found);
    CHECK(a.status == AlignStatus::Fallback);
    CHECK(a.transform.dx == 3 && a.transform.dy == 4 && a.transform.angle_deg == -0.5f && a.transform.scale == 1.01f);
  }
}

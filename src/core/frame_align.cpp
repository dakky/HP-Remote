#include "frame_align.h"

#include <math.h>
#include <string.h>
#include <vector>

// ── Suchbereiche (Anteile der Bildgröße, gemessen am WPT-Display in Referenzlage)
// Obere Linie bei ≈ 0,37·H, untere bei ≈ 0,78·H, senkrechte bei ≈ 0,50·W.
// Die Bänder decken die zulässige Verschiebung (±40 px bei 640×480), Drehung
// (±5°) und den Maßstab (0,9–1,1) ab.
static const float kTopBandFrom    = 0.22f;
static const float kTopBandTo      = 0.52f;
static const float kBottomBandFrom = 0.62f;
static const float kBottomBandTo   = 0.93f;
static const float kVertBandFrom   = 0.38f;
static const float kVertBandTo     = 0.62f;

// Messstreifen für die waagerechten Linien (Mitte als Anteil der Breite).
// Innerhalb der Linienenden auch bei maximaler Verschiebung und Maßstab 0,9.
static const float kStripLeft  = 0.22f;
static const float kStripRight = 0.78f;

// Mindestkontrast (Graustufen) zwischen Linie und Umgebung
static const float kMinContrast = 60.0f;

// Linien, die steiler als die zulässige Drehung (+1° Reserve) liegen, oder
// obere und untere Linie, deren Winkel stark voneinander abweichen, gelten
// als Fehlerkennung.
static const float kMaxSlope      = 0.105f;   // tan(6°)
static const float kMaxSlopeDiff  = 0.035f;   // tan(2°)

// Abstand der Vergleichspixel vom Linienzentrum. Die Linien sind ≈ 4 px dick
// (bei 800×600), Ziffernsegmente ≥ 9 px: Bei diesem Abstand ist neben einer
// Linie schon Hintergrund, neben der Mitte eines Segments noch Segment.
// Liefert die 4 px der Referenzauflösung (refSize) umgerechnet auf size.
static int lineSideDistance(int size, int refSize) {
  int d = (size * 4 + refSize / 2) / refSize;
  return d < 2 ? 2 : d;
}

// Linienfilter: hell in der Mitte, dunkel daneben (beide Seiten).
// Sucht in profile[from..to] die stärkste Antwort und liefert die Position
// subpixelgenau (Parabel durch das Maximum). false, wenn der Kontrast fehlt.
static bool findLine(const float *profile, int n, int from, int to, int d, float &pos) {
  if (from < d + 1) from = d + 1;
  if (to > n - d - 2) to = n - d - 2;
  if (from > to) return false;

  int best = -1;
  float bestR = 0;
  for (int i = from; i <= to; i++) {
    float side = profile[i - d] > profile[i + d] ? profile[i - d] : profile[i + d];
    float r = profile[i] - side;
    if (r > bestR) {
      bestR = r;
      best = i;
    }
  }
  if (best < 0 || bestR < kMinContrast) return false;

  // Subpixel: Parabel durch das Profil um das Maximum
  const float a = profile[best - 1];
  const float b = profile[best];
  const float c = profile[best + 1];
  const float den = a - 2.0f * b + c;
  float off = den < 0 ? 0.5f * (a - c) / den : 0.0f;
  if (off > 0.5f) off = 0.5f;
  if (off < -0.5f) off = -0.5f;
  pos = best + off;
  return true;
}

// Mittelwert jeder Zeile in einem schmalen senkrechten Streifen
static void rowProfile(const uint8_t *gray, int W, int H, int x0, int x1, float *profile) {
  const float inv = 1.0f / (x1 - x0);
  for (int y = 0; y < H; y++) {
    const uint8_t *row = gray + y * W;
    int sum = 0;
    for (int x = x0; x < x1; x++) sum += row[x];
    profile[y] = sum * inv;
  }
}

// Eine waagerechte Linie in beiden Streifen messen
static bool measureHLine(const float *profL, const float *profR, int H, float bandFrom, float bandTo,
                         float xl, float xr, AnchorPoint &left, AnchorPoint &right) {
  const int d = lineSideDistance(H, 600);
  const int from = (int)(bandFrom * H);
  const int to = (int)(bandTo * H);
  float yl, yr;
  if (!findLine(profL, H, from, to, d, yl)) return false;
  if (!findLine(profR, H, from, to, d, yr)) return false;
  if (fabsf(yr - yl) > kMaxSlope * (xr - xl)) return false;
  left = {xl, yl};
  right = {xr, yr};
  return true;
}

AnchorError measureAnchorLines(const uint8_t *gray, int W, int H, AnchorLines &out) {
  if (!gray || W < 64 || H < 48) return AnchorError::InvalidImage;

  // Streifenbreite: schmal genug, dass eine um 5° gedrehte Linie darin kaum
  // breiter wird (≈ 1 px bei 800×600)
  int sw = W / 80;
  if (sw < 2) sw = 2;
  const int xl0 = (int)(kStripLeft * W) - sw / 2;
  const int xr0 = (int)(kStripRight * W) - sw / 2;
  const float xl = xl0 + sw / 2.0f;
  const float xr = xr0 + sw / 2.0f;

  std::vector<float> profL(H), profR(H);
  rowProfile(gray, W, H, xl0, xl0 + sw, profL.data());
  rowProfile(gray, W, H, xr0, xr0 + sw, profR.data());

  AnchorLines l;
  if (!measureHLine(profL.data(), profR.data(), H, kTopBandFrom, kTopBandTo, xl, xr, l.top_left, l.top_right))
    return AnchorError::TopLine;
  if (!measureHLine(profL.data(), profR.data(), H, kBottomBandFrom, kBottomBandTo, xl, xr, l.bottom_left,
                    l.bottom_right))
    return AnchorError::BottomLine;

  const float slopeTop = (l.top_right.y - l.top_left.y) / (xr - xl);
  const float slopeBottom = (l.bottom_right.y - l.bottom_left.y) / (xr - xl);
  if (fabsf(slopeTop - slopeBottom) > kMaxSlopeDiff) return AnchorError::Inconsistent;

  // Senkrechte Linie: Spaltenmittel über die mittlere Hälfte zwischen den
  // waagerechten Linien. Jede Zeile wird um die gemessene Drehung versetzt
  // abgetastet, damit eine gedrehte Linie im Profil schmal bleibt
  // (Drehung im Uhrzeigersinn: senkrechte Linie wandert nach unten nach links).
  const float slope = 0.5f * (slopeTop + slopeBottom);
  const float yTop = 0.5f * (l.top_left.y + l.top_right.y);
  const float yBottom = 0.5f * (l.bottom_left.y + l.bottom_right.y);
  const float yMid = 0.5f * (yTop + yBottom);
  const float gap = yBottom - yTop;
  if (gap < 0.2f * H) return AnchorError::Inconsistent;

  const int y0 = (int)(yMid - gap / 4);
  const int y1 = (int)(yMid + gap / 4);
  const int dx = lineSideDistance(W, 800);
  const int maxShift = (int)ceilf(fabsf(slope) * gap / 4) + 1;
  const int c0 = (int)(kVertBandFrom * W) - dx - 1;
  const int c1 = (int)(kVertBandTo * W) + dx + 2;   // exklusiv

  // Spaltenprofil nur im Suchband (plus Rand für den Linienfilter);
  // die Zeilenversätze bleiben durch maxShift im Bild
  if (c0 - maxShift < 0 || c1 + maxShift > W) return AnchorError::VerticalLine;
  const int n = c1 - c0;
  std::vector<float> profV(n, 0.0f);
  for (int y = y0; y <= y1; y++) {
    const int shift = (int)lroundf(-(y - yMid) * slope);
    const uint8_t *row = gray + y * W + c0 + shift;
    for (int i = 0; i < n; i++) profV[i] += row[i];
  }
  const float inv = 1.0f / (y1 - y0 + 1);
  for (int i = 0; i < n; i++) profV[i] *= inv;

  float xv;
  if (!findLine(profV.data(), n, 0, n - 1, dx, xv)) return AnchorError::VerticalLine;

  l.vertical = {c0 + xv, yMid};
  out = l;
  return AnchorError::None;
}

const char *anchorErrorText(AnchorError err) {
  switch (err) {
    case AnchorError::None:         return "ok";
    case AnchorError::InvalidImage: return "no usable camera image";
    case AnchorError::TopLine:      return "upper horizontal anchor line not found";
    case AnchorError::BottomLine:   return "lower horizontal anchor line not found";
    case AnchorError::VerticalLine: return "vertical anchor line not found";
    case AnchorError::Inconsistent: return "anchor lines found but do not fit together";
  }
  return "unknown error";
}

// ── Ausrichten ────────────────────────────────────────────────────────────

static const float kDegToRad = 3.14159265f / 180.0f;

// Plausibilitätsgrenzen: Größere Abweichungen gelten als Fehlerkennung.
// Die Verschiebung ist für 640 px Breite angegeben und wird auf die
// tatsächliche Breite umgerechnet.
static const float kMaxShift640 = 40.0f;
static const float kMaxAngleDeg = 5.0f;
static const float kMinScale    = 0.9f;
static const float kMaxScale    = 1.1f;

static bool isPlausible(const AlignTransform &t, int W) {
  const float maxShift = kMaxShift640 * W / 640.0f;
  return fabsf(t.dx) <= maxShift && fabsf(t.dy) <= maxShift && fabsf(t.angle_deg) <= kMaxAngleDeg &&
         t.scale >= kMinScale && t.scale <= kMaxScale;
}

// Lage des Displays aus den Ankerlinien: Steigung der waagerechten Linien,
// senkrechter Abstand zwischen ihnen und ein fester Punkt (Schnitt der
// senkrechten Linie mit der Mittellinie zwischen den waagerechten Linien).
struct DisplayPose {
  float slope;
  float gap;
  AnchorPoint center;
};

static DisplayPose poseFromLines(const AnchorLines &l) {
  // Beide Linien werden in denselben Streifen gemessen (gleiche x-Stellen)
  const float x0 = l.top_left.x;
  const float x1 = l.top_right.x;
  const float m0 = 0.5f * (l.top_left.y + l.bottom_left.y);
  const float m1 = 0.5f * (l.top_right.y + l.bottom_right.y);
  const float k = (m1 - m0) / (x1 - x0);
  const float dy = 0.5f * ((l.bottom_left.y - l.top_left.y) + (l.bottom_right.y - l.top_right.y));
  const float norm = sqrtf(1.0f + k * k);

  // Senkrechte Linie steht rechtwinklig auf den waagerechten: Richtung (−k, 1).
  // Schnitt mit der Mittellinie y = m0 + k·(x − x0):
  //   V.y + t = m0 + k·(V.x − t·k − x0)
  const AnchorPoint &v = l.vertical;
  const float t = (m0 + k * (v.x - x0) - v.y) / (norm * norm);

  DisplayPose p;
  p.slope = k;
  p.gap = dy / norm;
  p.center = {v.x - t * k, v.y + t};
  return p;
}

AlignResult estimateAlignment(const uint8_t *gray, int W, int H, const AnchorLines *ref,
                              const AlignTransform &lastValid) {
  AlignResult r = {};
  r.transform = kAlignIdentity;
  r.error = AnchorError::None;
  if (!ref) {
    r.status = AlignStatus::NoReference;
    return r;
  }

  r.error = measureAnchorLines(gray, W, H, r.lines);
  r.lines_found = r.error == AnchorError::None;
  if (!r.lines_found) {
    r.status = AlignStatus::Fallback;
    r.transform = lastValid;
    return r;
  }

  const DisplayPose pr = poseFromLines(*ref);
  const DisplayPose pc = poseFromLines(r.lines);
  const float rad = atanf(pc.slope) - atanf(pr.slope);
  const float s = pc.gap / pr.gap;

  // Verschiebung so, dass der Referenzpunkt auf den gemessenen Punkt fällt:
  //   pc = c + s·R·(pr − c) + d
  const float cx = W / 2.0f;
  const float cy = H / 2.0f;
  const float cs = cosf(rad);
  const float sn = sinf(rad);
  const float qx = pr.center.x - cx;
  const float qy = pr.center.y - cy;

  AlignTransform t;
  t.angle_deg = rad / kDegToRad;
  t.scale = s;
  t.dx = pc.center.x - cx - s * (cs * qx - sn * qy);
  t.dy = pc.center.y - cy - s * (sn * qx + cs * qy);

  if (!isPlausible(t, W)) {
    r.status = AlignStatus::Fallback;
    r.transform = lastValid;
    return r;
  }
  r.status = AlignStatus::Ok;
  r.transform = t;
  return r;
}

void applyAlignment(uint8_t *gray, int W, int H, const AlignTransform &t, uint8_t *scratch) {
  if (!gray || !scratch || W <= 0 || H <= 0) return;
  // Unter 0,05 px bzw. 0,05° nicht lohnenswert (wie die Feinrotation)
  if (fabsf(t.dx) < 0.05f && fabsf(t.dy) < 0.05f && fabsf(t.angle_deg) < 0.05f &&
      fabsf(t.scale - 1.0f) < 0.0001f)
    return;

  memcpy(scratch, gray, (size_t)W * H);

  // Rückwärts: Zielpixel q (Referenzlage) holt sich das Quellpixel
  //   p = c + s·R·(q − c) + d
  // Entlang einer Zeile wächst p linear mit x → pro Pixel nur zwei Additionen.
  const float rad = t.angle_deg * kDegToRad;
  const float a = t.scale * cosf(rad);
  const float b = t.scale * sinf(rad);
  const float cx = W / 2.0f;
  const float cy = H / 2.0f;

  for (int y = 0; y < H; y++) {
    const float qy = y - cy;
    // + 0,5 für Runden beim Abschneiden; Quellpositionen < 0 fallen über
    // den Bereichstest auf sx/sy ≥ 0 heraus
    float sx = cx + t.dx + a * (-cx) - b * qy + 0.5f;
    float sy = cy + t.dy + b * (-cx) + a * qy + 0.5f;
    uint8_t *row = gray + y * W;
    for (int x = 0; x < W; x++, sx += a, sy += b) {
      uint8_t val = 0;
      if (sx >= 0.0f && sy >= 0.0f) {
        const int ix = (int)sx;
        const int iy = (int)sy;
        if (ix < W && iy < H) val = scratch[iy * W + ix];
      }
      row[x] = val;
    }
  }
}

const char *alignStatusText(AlignStatus status) {
  switch (status) {
    case AlignStatus::Ok:          return "ok";
    case AlignStatus::Fallback:    return "fallback";
    case AlignStatus::NoReference: return "no_reference";
  }
  return "unknown";
}

const char *alignFallbackText(const AlignResult &r) {
  if (r.status != AlignStatus::Fallback) return nullptr;
  return r.lines_found ? "anchor lines outside plausible range" : anchorErrorText(r.error);
}

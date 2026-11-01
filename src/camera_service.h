#pragma once

#include <Arduino.h>
#include "esp_camera.h"
#include "app_config.h"
#include "frame_align.h"

// Anzahl der Framebuffer – muss mit c.fb_count in cameraStart() übereinstimmen.
// Beim Capture müssen genau CAMERA_FB_COUNT Dummy-Frames verworfen werden,
// damit bei Inaktivität keine veralteten Frames zurückgegeben werden.
//
static constexpr int CAMERA_FB_COUNT = 2;

bool cameraStart(const AppConfig &cfg);
camera_fb_t* cameraCapture();
camera_fb_t* cameraCaptureFast();   // Schnell, ohne Dummy-Drop – für Stream
void cameraRelease(camera_fb_t *fb);
void cameraPrepareFrame(camera_fb_t *fb, const AppConfig &cfg);   // Auto Align bzw. Feinrotation vor jeder Nutzung
// Ergebnis von Auto Align im zuletzt vorbereiteten Bild (nur im Speicher).
// enabled == false: Auto Align war aus, result ist dann ohne Bedeutung.
struct CameraAlignInfo {
  bool enabled;
  AlignResult result;
  float detect_ms;   // Linien messen und Transformation bestimmen
  float align_ms;    // Bild ausrichten
};
const CameraAlignInfo &cameraAlignInfo();

bool cameraToJpeg(camera_fb_t *fb, int quality, uint8_t **jpg_buf, size_t *jpg_len);
bool cameraToJpegScaled(camera_fb_t *fb, int quality, uint8_t **jpg_buf, size_t *jpg_len);

bool cameraRestart(const AppConfig &cfg);
bool cameraIsHealthy();
void cameraMarkCaptureFailure();
void cameraMarkCaptureSuccess();
unsigned long cameraCaptureFailureCount();

const char* cameraSensorName();
bool cameraApplySettings(const AppConfig &cfg);
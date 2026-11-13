// Tests der Konfiguration über ihr Interface: Defaults laden, Config → JSON,
// JSON → Config. Die Default-Namen kommen aus der Test-MAC 02:00:00:ab:cd:ef
// des Host-Fakes (fake_mac.cpp).
#include "test.h"

#include "core/app_config.h"

#include <string>

static AppConfig defaults() {
  AppConfig cfg;
  loadDefaultConfig(cfg);
  return cfg;
}

TEST(Config_Defaults_Namen_kommen_aus_der_MAC) {
  const AppConfig cfg = defaults();

  CHECK(cfg.device.id == "hp_remote_abcdef");
  CHECK(cfg.device.name == "HP-Remote abcdef");
  CHECK(cfg.wifi.hostname == "hp-remote-abcdef");
  CHECK(cfg.mqtt.topic == "heatpump/hp_remote_abcdef/state");
}

TEST(Config_Defaults_Werte) {
  const AppConfig cfg = defaults();

  CHECK(cfg.wifi.ssid == "");
  CHECK(cfg.wifi.password == "");

  CHECK(cfg.camera.width == 800);
  CHECK(cfg.camera.height == 600);
  CHECK(cfg.camera.jpeg_quality == 10);
  CHECK(cfg.camera.auto_exposure);
  CHECK(cfg.camera.auto_gain);
  CHECK(cfg.camera.auto_whitebalance);
  CHECK(cfg.camera.aec_value == 300);
  CHECK(cfg.camera.agc_gain == 0);
  CHECK(!cfg.camera.vflip);
  CHECK(!cfg.camera.hflip);
  CHECK(cfg.camera.fine_rotation == 0.0f);
  CHECK(!cfg.camera.auto_align);
  CHECK(!cfg.camera.align_ref_set);

  CHECK(cfg.capture_interval_sec == 60);
  CHECK(cfg.debug_level == 1);

  CHECK(!cfg.mqtt.enabled);
  CHECK(!cfg.mqtt.image_enabled);
  CHECK(cfg.mqtt.host == "192.168.1.10");
  CHECK(cfg.mqtt.port == 1883);
  CHECK(cfg.mqtt.username == "");
  CHECK(cfg.mqtt.password == "");

  CHECK(cfg.ha.enabled);
  CHECK(cfg.ha.discovery_prefix == "homeassistant");
  CHECK(cfg.stream.enabled);

  CHECK(cfg.room_sensor.enabled);
  CHECK(cfg.room_sensor.temp_offset == 0.0f);
  CHECK(cfg.room_sensor.humidity_offset == 0.0f);
  CHECK(cfg.room_sensor.read_interval_sec == 60);
}

TEST(Config_Defaults_Segment_Profil_und_ROIs) {
  const AppConfig cfg = defaults();

  REQUIRE(cfg.seg_profiles.size() == 1);
  CHECK(cfg.seg_profiles[0].name == "standard");
  CHECK(cfg.seg_profiles[0].segs[0].rx == 0.36f);  // a oben
  CHECK(cfg.seg_profiles[0].segs[6].ry == 0.47f);  // g Mitte

  REQUIRE(cfg.rois.size() == 2);
  const Roi &v = cfg.rois[0];
  CHECK(v.id == "display_value");
  CHECK(v.label == "Display Value");
  CHECK(v.type == "sevenseg");
  CHECK(v.x == 140);
  CHECK(v.y == 70);
  CHECK(v.w == 170);
  CHECK(v.h == 70);
  CHECK(v.threshold == 105);
  CHECK(v.digit_gap_px == 8);
  CHECK(v.digits == 4);
  CHECK(v.decimal_places == 0);
  CHECK(v.gaps.empty());
  CHECK(v.threshold_on == 125);
  CHECK(v.threshold_off == 105);
  CHECK(v.ha_enabled);
  CHECK(v.seg_profile == "standard");
  CHECK(!v.locked);

  const Roi &s = cfg.rois[1];
  CHECK(s.id == "symbol_1");
  CHECK(s.type == "symbol");
  CHECK(s.digits == 2);
}

// Jedes Feld weicht vom Default ab. Gleitkommawerte sind binär exakt
// darstellbar, damit der Vergleich nach dem Roundtrip exakt sein darf.
static AppConfig customized() {
  AppConfig cfg = defaults();

  cfg.device.name = "Keller";
  cfg.wifi.ssid = "netz";
  cfg.wifi.password = "geheim";

  cfg.camera.width = 640;
  cfg.camera.height = 480;
  cfg.camera.jpeg_quality = 20;
  cfg.camera.auto_exposure = false;
  cfg.camera.auto_gain = false;
  cfg.camera.auto_whitebalance = false;
  cfg.camera.aec_value = 500;
  cfg.camera.agc_gain = 7;
  cfg.camera.vflip = true;
  cfg.camera.hflip = true;
  cfg.camera.fine_rotation = -2.5f;
  cfg.camera.auto_align = true;
  cfg.camera.align_ref_set = true;
  cfg.camera.align_ref.top_left = {176.0f, 219.5f};
  cfg.camera.align_ref.top_right = {624.0f, 220.25f};
  cfg.camera.align_ref.bottom_left = {176.0f, 466.0f};
  cfg.camera.align_ref.bottom_right = {624.0f, 465.5f};
  cfg.camera.align_ref.vertical = {402.75f, 342.0f};

  cfg.capture_interval_sec = 30;
  cfg.debug_level = 3;

  cfg.mqtt.enabled = true;
  cfg.mqtt.image_enabled = true;
  cfg.mqtt.host = "broker.example";
  cfg.mqtt.port = 8883;
  cfg.mqtt.username = "nutzer";
  cfg.mqtt.password = "pw";

  cfg.ha.enabled = false;
  cfg.ha.discovery_prefix = "ha";
  cfg.stream.enabled = false;

  cfg.room_sensor.enabled = false;
  cfg.room_sensor.temp_offset = -1.5f;
  cfg.room_sensor.humidity_offset = 2.25f;
  cfg.room_sensor.read_interval_sec = 120;

  SevenSegProfile p;
  p.name = "schmal";
  for (int i = 0; i < 7; i++) p.segs[i] = {0.125f * i, 0.25f, 0.5f, 0.0625f};
  cfg.seg_profiles.push_back(p);

  Roi r = cfg.rois[0];
  r.id = "water_temp";
  r.label = "Wasser";
  r.type = "sevenseg";
  r.x = 11;
  r.y = 22;
  r.w = 33;
  r.h = 44;
  r.threshold = 90;
  r.digit_gap_px = 5;
  r.digits = 3;
  r.decimal_places = 1;
  r.gaps = {4, 6};
  r.threshold_on = 130;
  r.threshold_off = 80;
  r.invert_logic = true;
  r.auto_threshold = true;
  r.stretch_contrast = true;
  r.confidence_margin = 12;
  r.ha_enabled = false;
  r.ha_name = "Wassertemperatur";
  r.ha_unit = "°C";
  r.ha_icon = "mdi:water";
  r.ha_device_class = "temperature";
  r.ha_state_class = "measurement";
  r.seg_profile = "schmal";
  r.locked = true;
  cfg.rois = {r};

  return cfg;
}

static void checkSamePoint(const AnchorPoint &a, const AnchorPoint &b) {
  CHECK(a.x == b.x);
  CHECK(a.y == b.y);
}

TEST(Config_Roundtrip_erhaelt_alle_Felder) {
  const AppConfig in = customized();
  AppConfig out = defaults();

  REQUIRE(configFromJson(configToJson(in), out));

  CHECK(out.device.id == "hp_remote_abcdef");
  CHECK(out.device.name == "Keller");
  CHECK(out.wifi.ssid == "netz");
  CHECK(out.wifi.password == "geheim");
  CHECK(out.wifi.hostname == "hp-remote-abcdef");

  CHECK(out.camera.width == 640);
  CHECK(out.camera.height == 480);
  CHECK(out.camera.jpeg_quality == 20);
  CHECK(!out.camera.auto_exposure);
  CHECK(!out.camera.auto_gain);
  CHECK(!out.camera.auto_whitebalance);
  CHECK(out.camera.aec_value == 500);
  CHECK(out.camera.agc_gain == 7);
  CHECK(out.camera.vflip);
  CHECK(out.camera.hflip);
  CHECK(out.camera.fine_rotation == -2.5f);
  CHECK(out.camera.auto_align);
  CHECK(out.camera.align_ref_set);
  checkSamePoint(out.camera.align_ref.top_left, in.camera.align_ref.top_left);
  checkSamePoint(out.camera.align_ref.top_right, in.camera.align_ref.top_right);
  checkSamePoint(out.camera.align_ref.bottom_left, in.camera.align_ref.bottom_left);
  checkSamePoint(out.camera.align_ref.bottom_right, in.camera.align_ref.bottom_right);
  checkSamePoint(out.camera.align_ref.vertical, in.camera.align_ref.vertical);

  CHECK(out.capture_interval_sec == 30);
  CHECK(out.debug_level == 3);

  CHECK(out.mqtt.enabled);
  CHECK(out.mqtt.image_enabled);
  CHECK(out.mqtt.host == "broker.example");
  CHECK(out.mqtt.port == 8883);
  CHECK(out.mqtt.topic == "heatpump/hp_remote_abcdef/state");
  CHECK(out.mqtt.username == "nutzer");
  CHECK(out.mqtt.password == "pw");

  CHECK(!out.ha.enabled);
  CHECK(out.ha.discovery_prefix == "ha");
  CHECK(!out.stream.enabled);

  CHECK(!out.room_sensor.enabled);
  CHECK(out.room_sensor.temp_offset == -1.5f);
  CHECK(out.room_sensor.humidity_offset == 2.25f);
  CHECK(out.room_sensor.read_interval_sec == 120);

  REQUIRE(out.seg_profiles.size() == 2);
  CHECK(out.seg_profiles[0].name == "standard");
  CHECK(out.seg_profiles[1].name == "schmal");
  for (int i = 0; i < 7; i++) {
    CHECK(out.seg_profiles[1].segs[i].rx == 0.125f * i);
    CHECK(out.seg_profiles[1].segs[i].ry == 0.25f);
    CHECK(out.seg_profiles[1].segs[i].rw == 0.5f);
    CHECK(out.seg_profiles[1].segs[i].rh == 0.0625f);
  }

  REQUIRE(out.rois.size() == 1);
  const Roi &r = out.rois[0];
  CHECK(r.id == "water_temp");
  CHECK(r.label == "Wasser");
  CHECK(r.type == "sevenseg");
  CHECK(r.x == 11);
  CHECK(r.y == 22);
  CHECK(r.w == 33);
  CHECK(r.h == 44);
  CHECK(r.threshold == 90);
  CHECK(r.digit_gap_px == 5);
  CHECK(r.digits == 3);
  CHECK(r.decimal_places == 1);
  CHECK((r.gaps == std::vector<int>{4, 6}));
  CHECK(r.threshold_on == 130);
  CHECK(r.threshold_off == 80);
  CHECK(r.invert_logic);
  CHECK(r.auto_threshold);
  CHECK(r.stretch_contrast);
  CHECK(r.confidence_margin == 12);
  CHECK(!r.ha_enabled);
  CHECK(r.ha_name == "Wassertemperatur");
  CHECK(r.ha_unit == "°C");
  CHECK(r.ha_icon == "mdi:water");
  CHECK(r.ha_device_class == "temperature");
  CHECK(r.ha_state_class == "measurement");
  CHECK(r.seg_profile == "schmal");
  CHECK(r.locked);
}

TEST(Config_Import_ersetzt_Geraete_Namen_durch_die_aus_der_MAC) {
  AppConfig cfg = defaults();

  REQUIRE(configFromJson(R"({"device":{"id":"hp_remote_ffffff"},
                             "wifi":{"hostname":"fremd-host"},
                             "mqtt":{"topic":"heatpump/fremd/state"}})",
                         cfg));

  CHECK(cfg.device.id == "hp_remote_abcdef");
  CHECK(cfg.wifi.hostname == "hp-remote-abcdef");
  CHECK(cfg.mqtt.topic == "heatpump/hp_remote_abcdef/state");
}

// Wie beim Start: Defaults laden, dann die gespeicherte Config darüberlesen
TEST(Config_fehlende_Felder_behalten_den_Default) {
  AppConfig cfg = defaults();

  REQUIRE(configFromJson(R"({"mqtt":{"host":"broker.example"}})", cfg));

  CHECK(cfg.mqtt.host == "broker.example");
  CHECK(cfg.mqtt.port == 1883);
  CHECK(!cfg.mqtt.enabled);
  CHECK(cfg.device.name == "HP-Remote abcdef");
  CHECK(cfg.camera.width == 800);
  CHECK(cfg.camera.height == 600);
  CHECK(!cfg.camera.auto_align);
  CHECK(cfg.capture_interval_sec == 60);
  CHECK(cfg.debug_level == 1);
  CHECK(cfg.ha.discovery_prefix == "homeassistant");
  CHECK(cfg.room_sensor.read_interval_sec == 60);
  REQUIRE(cfg.seg_profiles.size() == 1);
  CHECK(cfg.seg_profiles[0].name == "standard");
  REQUIRE(cfg.rois.size() == 2);
  CHECK(cfg.rois[0].id == "display_value");
}

TEST(Config_Felder_mit_falschem_Typ_behalten_den_Default) {
  AppConfig cfg = defaults();

  REQUIRE(configFromJson(R"({"camera":{"width":"breit"},
                             "mqtt":{"host":5,"port":"x"},
                             "rois":"keine"})",
                         cfg));

  CHECK(cfg.camera.width == 800);
  CHECK(cfg.mqtt.host == "192.168.1.10");
  CHECK(cfg.mqtt.port == 1883);
  REQUIRE(cfg.rois.size() == 2);
  CHECK(cfg.rois[0].id == "display_value");
}

TEST(Config_fehlende_ROI_Felder_bekommen_den_Default) {
  AppConfig cfg = defaults();

  REQUIRE(configFromJson(R"({"rois":[{"id":"nur_id"}]})", cfg));

  REQUIRE(cfg.rois.size() == 1);
  const Roi &r = cfg.rois[0];
  CHECK(r.id == "nur_id");
  CHECK(r.label == "");
  CHECK(r.type == "sevenseg");
  CHECK(r.x == 0);
  CHECK(r.y == 0);
  CHECK(r.w == 1);
  CHECK(r.h == 1);
  CHECK(r.threshold == 105);
  CHECK(r.digit_gap_px == 8);
  CHECK(r.digits == 2);
  CHECK(r.decimal_places == 0);
  CHECK(r.gaps.empty());
  CHECK(r.threshold_on == 125);
  CHECK(r.threshold_off == 105);
  CHECK(!r.invert_logic);
  CHECK(r.ha_enabled);
  CHECK(r.seg_profile == "standard");
  CHECK(!r.locked);
}

TEST(Config_ungueltiges_JSON_meldet_Fehler_und_laesst_die_Config_unveraendert) {
  const AppConfig before = customized();
  AppConfig cfg = before;

  CHECK(!configFromJson(R"({"mqtt":{"host":"kaputt")", cfg));

  CHECK(configToJson(cfg) == configToJson(before));
  CHECK(cfg.mqtt.host == "broker.example");
  REQUIRE(cfg.rois.size() == 1);
  CHECK(cfg.rois[0].id == "water_temp");
}

// Tests des Logging-Moduls über den sammelnden Fake der Log-Ausgabe.
#include "test.h"
#include "fake_log_output.h"

#include "core/log.h"

#include <string>

TEST(Log_gibt_bis_zum_eingestellten_Level_mit_Praefix_aus) {
  logSetLevel(1);
  fakeLogTake();

  LOGE("kaputt %d\n", 7);
  LOGI("Info %s\n", "da");
  LOGD("Debug\n");
  LOGA("Analyse\n");

  CHECK(fakeLogTake() == "[ERR] kaputt 7\n[INF] Info da\n");
}

TEST(Log_gibt_auf_hoechstem_Level_alle_Level_mit_Praefix_aus) {
  logSetLevel(3);
  fakeLogTake();

  LOGE("E\n");
  LOGI("I\n");
  LOGD("D\n");
  LOGA("A\n");

  CHECK(fakeLogTake() == "[ERR] E\n[INF] I\n[DBG] D\n[ANL] A\n");
}

TEST(Log_gibt_lange_Texte_vollstaendig_aus) {
  logSetLevel(1);
  fakeLogTake();
  const std::string payload(1000, 'x');

  LOGI("%s\n", payload.c_str());

  CHECK(fakeLogTake() == "[INF] " + payload + "\n");
}

TEST(Log_gibt_auf_Level_0_nur_Fehler_aus) {
  logSetLevel(0);
  fakeLogTake();

  LOGE("Fehler\n");
  LOGI("Info\n");

  CHECK(fakeLogTake() == "[ERR] Fehler\n");
}

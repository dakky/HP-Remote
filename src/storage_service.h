#pragma once

#include <Arduino.h>
#include "core/app_config.h"

bool storageBegin();
bool loadConfigFromFile(AppConfig &cfg);
bool saveConfigToFile(const AppConfig &cfg);
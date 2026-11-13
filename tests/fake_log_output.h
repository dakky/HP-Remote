// Host-Fake der Log-Ausgabe: sammelt alles, was geloggt wird.
#pragma once

#include <string>

// Gibt die seit dem letzten Aufruf gesammelte Ausgabe zurück und leert sie.
std::string fakeLogTake();

#ifndef UTILS_H
#define UTILS_H

#include "MyESP.h"
#include "Arduino.h"

#define myDebug(...)    myESP.myDebug(__VA_ARGS__)
#define myDebug_P(...)  myESP.myDebug_P(__VA_ARGS__)

#endif //UTILS_H
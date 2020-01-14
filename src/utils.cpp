#include "string.h"
#include "utils.h"

static char _appVersion[20] = {'\0'};

bool hasValue(const char *s)
{
    if ((s == nullptr) || (strlen(s) == 0)) {
        return false;
    }
    return (s[0] != '\0');
}



char* getAppVersion()
{
    return &_appVersion[0];
}


// reset / restart
void resetESP() {
    myDebug_P(PSTR("* Restart ESP..."));
    //_setSystemBootStatus(MYESP_BOOTSTATUS_POWERON);
    //_setCustomResetReason(CUSTOM_RESET_TERMINAL);
    delay(500);
    
#if defined(ARDUINO_ARCH_ESP32)
    ESP.restart();
#else
    ESP.restart();
#endif
}

uint32_t getInitialFreeHeap() {
    static uint32_t _heap = 0;

    if (0 == _heap) {
        _heap = ESP.getFreeHeap();
    }

    return _heap;
}

// returns system uptime in seconds
unsigned long getUptime() {
    static uint32_t last_uptime      = 0;
    static uint8_t  uptime_overflows = 0;

    if (millis() < last_uptime) {
        ++uptime_overflows;
    }
    last_uptime             = millis();
    uint32_t uptime_seconds = uptime_overflows * (MYESP_UPTIME_OVERFLOW / 1000) + (last_uptime / 1000);

    return uptime_seconds;
}
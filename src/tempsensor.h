/*
 * Support for DHT11 temperature sensors
 */

#pragma once

#include "DHT.h"
// #define DHT_DEBUG  //DEBUGGING INFO
#define DHTPIN D7 //13 //D4
#define DHTTYPE DHT21

class TempSensor {
    public:
        TempSensor();
        ~TempSensor();
        void initialize();
        void process();
        float getTemperature();
        float getHumidity();
    private:
        DHT *pDht;
        float curr_temp;
        float curr_hum;
};


#ifndef THERMOSTAT_H_
#define THERMOSTAT_H_

#include <uuid/common.h>
#include <uuid/console.h>
#include <uuid/log.h>
#include "tempsensor.h"
#include "display.h"

namespace emsesp {

class ThermostatDevice {

public:
    void start();
    void loop();

private:
    
    //Structures
    typedef struct {
        uint32_t processUpdateCtr;
        uint32_t tempUpdateValue;
    } ProcessUpdates;

    typedef struct {
        float temperature;
        float humidity;
        float relativeTemp;
    } TempSensorValue;

    typedef struct {
        uint32_t timestamp;      // for internal timings, via millis()
        TempSensor tempSensor;
        Display display;
        float setPointTemp;
        ProcessUpdates pUpdate;
        TempSensorValue SVcurrent;
        TempSensorValue SVprevious;
    } Admin;

    //Functions
    bool compareSensorValues(TempSensorValue *current, TempSensorValue *previous, float diff);

    //Variables;
    Admin mThermostat;
    static uuid::log::Logger logger_;
};
} //namespace 
#endif //THERMOSTAT_H_
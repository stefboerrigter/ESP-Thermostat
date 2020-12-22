#include "thermostat.h"

namespace emsesp {

void ThermostatDevice::loop()
{
    if(mThermostat.pUpdate.processUpdateCtr++ % mThermostat.pUpdate.tempUpdateValue == 0)
    {
        mThermostat.tempSensor.process();
        mThermostat.SVcurrent.temperature = mThermostat.tempSensor.getTemperature();
        mThermostat.SVcurrent.humidity = mThermostat.tempSensor.getHumidity();
        mThermostat.SVcurrent.relativeTemp = mThermostat.tempSensor.getRelTemp();
        mThermostat.display.process(mThermostat.SVcurrent.temperature , 
            mThermostat.setPointTemp, mThermostat.SVcurrent.humidity, 
            mThermostat.SVcurrent.relativeTemp);
        if(compareSensorValues(&mThermostat.SVcurrent, &mThermostat.SVprevious, 0.1F))
        {
            //shell.printfln("MQTT UPDATE");
        }
        //Copy the current values to the previous.
        memcpy_P(&mThermostat.SVprevious, &mThermostat.SVcurrent, sizeof(TempSensorValue));
    }
}

void ThermostatDevice::start()
{
    mThermostat.pUpdate.tempUpdateValue = 500; //every Nth iteration
    mThermostat.pUpdate.processUpdateCtr = 0;

    mThermostat.setPointTemp = 20.0;
    memset(&(mThermostat.SVprevious), 0, sizeof(TempSensorValue));

    mThermostat.tempSensor.initialize();
    mThermostat.display.initialize();
}


bool ThermostatDevice::compareSensorValues(TempSensorValue *current, TempSensorValue *previous, float diff)
{
    if((current->temperature < (previous->temperature - diff)) || 
       (current->temperature > (previous->temperature + diff)))
    {
        return true;
    }
    if((current->humidity < (previous->humidity - diff)) || 
       (current->humidity > (previous->humidity + diff)))
    {
        return true;
    }
    //No need to check the relative temp, since if both changed, this changed as well.

    return false; //Match
}

} // namespace emsesp
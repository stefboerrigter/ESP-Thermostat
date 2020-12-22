#include "tempsensor.h"

namespace emsesp {

//uuid::log::Logger TempSensor::logger_{F_(tempsensor), uuid::log::Facility::DAEMON};

TempSensor::TempSensor(){
    pDht = new DHT(DHTPIN, DHTTYPE);
    curr_hum = 200.0;
    curr_temp = 99.0;
    curr_rel_temp = 200.0;
}

TempSensor::~TempSensor(){

}

void TempSensor::initialize()
{
    pDht->begin();
    //LOG_INFO(F("[TEMP] Initialized"));
}

void TempSensor::process()
{
  float hic;
  float h = pDht->readHumidity();
  float t = pDht->readTemperature();

  if (isnan(h) || isnan(t)) {
    //shell.printfln(PSTR("Failed to read from DHT sensor! Exit"));
    //LOG_INFO(F("[TEMP] Failed to read from DHT sensor! Exit"));
    return;
  }
  curr_temp = t;
  curr_hum = h;
  
  hic = pDht->computeHeatIndex(t, h, false);

  curr_rel_temp = hic;

  //shell.printfln(PSTR("Hum %d | Temp %d °C| Ind: %d °C"), (int)h, (int)t, (int)hic);
}

float TempSensor::getRelTemp()
{
    return curr_rel_temp;
}

float TempSensor::getHumidity()
{
    return curr_hum;
}

float TempSensor::getTemperature()
{
    return curr_temp;
}

} //namespace
#include "tempsensor.h"
#include "utils.h"

TempSensor::TempSensor(){
    pDht = new DHT(DHTPIN, DHTTYPE);
    curr_hum = 200.0;
    curr_temp = 99.0;
}

TempSensor::~TempSensor(){

}

void TempSensor::initialize()
{
    pDht->begin();
    myDebug_P(PSTR("[TEMP] Initialized"));
}

void TempSensor::process()
{
  float hic;
  float h = pDht->readHumidity();
  float t = pDht->readTemperature();

  if (isnan(h) || isnan(t)) {
    myDebug_P(PSTR("Failed to read from DHT sensor! Exit"));
    return;
  }
  curr_temp = t;
  curr_hum = h;
  
  hic = pDht->computeHeatIndex(t, h, false);

  myDebug_P(PSTR("Hum %d | Temp %d °C| Ind: %d °C"), (int)h, (int)t, (int)hic);
}

float TempSensor::getHumidity()
{
    return curr_hum;
}

float TempSensor::getTemperature()
{
    return curr_temp;
}
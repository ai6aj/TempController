#include <OneWire.h>
#include "TempSensor.h"

TempSensor::TempSensor(int pin) {
  ds = new OneWire(pin);
  numSensors = doSensorSearch();
  lastTemp = 0;
  timesRead = 0;
  lastTimeRead = 0;
  lastReqSent = 0;
}

void TempSensor::sendRequest(int sensor) {
  ds->reset();
  ds->select(&sensors[sensor*8]);
  ds->write(0x44,1);         // start conversion, with parasite power on at the end
  lastReqSent = millis();  
}

void TempSensor::loop() {
  if (millis() - lastReqSent > 750) {
    if (lastReqSent != 0) {
      lastTemp = getTemperatureAsync(0);
      timesRead++;
    }
    sendRequest(0);
  }
}

int TempSensor::isAsyncReady(int sensor) {
  return (lastTimeRead != timesRead);
}

int TempSensor::getAsyncTemperature(int sensor) {
  lastTimeRead = timesRead;
  return lastTemp;
}


int TempSensor::getTemperatureAsync(int sensor) {
  byte present = ds->reset();
  ds->select(&sensors[sensor*8]);    
  ds->write(0xBE);         // Read Scratchpad

  for ( int i = 0; i < 9; i++) {           // we need 9 bytes
    sensorData[i] = ds->read();
  }

  long temp = (long)(((int)sensorData[1] << 8) + (int)sensorData[0]);
  
  temp *= 625;
  temp *= 90;
  temp /= 50;
  temp += 320000;
  
  return (temp)/1000;
}

int TempSensor::doSensorSearch() {
   memset(sensors,0,MAX_TEMP_SENSORS*8);
   ds->reset_search();
   int numFound =0;
   while (numFound<MAX_TEMP_SENSORS && ds->search(&sensors[numFound*8])) numFound++;
   ds->reset_search();
   return numFound;
}

int TempSensor::getTemperature(int sensor) {
  ds->reset();
  ds->select(&sensors[sensor*8]);
  ds->write(0x44,1);         // start conversion, with parasite power on at the end

  delay(750);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  byte present = ds->reset();
  ds->select(&sensors[sensor*8]);    
  ds->write(0xBE);         // Read Scratchpad

  for ( int i = 0; i < 9; i++) {           // we need 9 bytes
    sensorData[i] = ds->read();
  }

  long temp = (long)(((int)sensorData[1] << 8) + (int)sensorData[0]);
  
  temp *= 625;
  temp *= 90;
  temp /= 50;
  temp += 320000;
  
  return (temp+500)/1000;
}

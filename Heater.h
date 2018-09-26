#ifndef __HEATER_H__
#define __HEATER_H__
#include "TempSensor.h"

class Heater {
  public:
    Heater(int relay_pin, TempSensor* tempSensorOW);
    void setTargetTemp(int what);
    int getTargetTemp();
    int getTemp();
    void setPowerState(int what);
    int getPowerState();
    int getHeaterState();
    void loop();

  protected:
    TempSensor* sensor;
    int relayPin;
    int targetTemp;
    int lastTemp;
    int powerState;
    int heaterState;
    void heaterOn();
    void heaterOff();
};

#endif
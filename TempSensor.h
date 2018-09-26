#ifndef __TEMPSENSOR_H__
#define __TEMPSENSOR_H__
#include <OneWire.h>
#define MAX_TEMP_SENSORS 2


//float lastTemp = -100;
//int pwm = 255;

class TempSensor {
  public:
    TempSensor(int pin);
    int getTemperature(int sensor);
    int isAsyncReady(int sensor);
    int getAsyncTemperature(int sensor);
    
    void loop();
    
  protected:
    byte sensors[MAX_TEMP_SENSORS*8];
    byte sensorData[12];
    int numSensors;
    long lastReqSent;
    int timesRead;
    int lastTimeRead;
    int lastTemp;
    
    OneWire* ds;
    int doSensorSearch();
    void sendRequest(int sensor);
    int getTemperatureAsync(int sensor);
};


#endif

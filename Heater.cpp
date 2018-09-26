#include "Heater.h"

Heater::Heater(int relay_pin,TempSensor* tempSensor) {
  lastTemp = 0;
  targetTemp = 0;
  powerState = 0;
  heaterState = 0;
  relayPin = relay_pin;
  sensor = tempSensor;
  pinMode(relayPin,OUTPUT);
}

void Heater::setTargetTemp(int what) {
  targetTemp = what;
}

void Heater::setPowerState(int what) {
  if (powerState && !what) {
    heaterOff();
  }
  powerState = what;
}

int Heater::getTargetTemp() {
  return targetTemp;
}

int Heater::getPowerState() {
  return powerState;
}

int Heater::getHeaterState() {
  return heaterState;
}

void Heater::heaterOn() {
  // turn on the relay
  digitalWrite(relayPin,1);
  heaterState = 1;
}

void Heater::heaterOff() {
  // turn off the relay
  digitalWrite(relayPin,0);
  heaterState = 0;
}

void Heater::loop() {
    lastTemp = sensor->getAsyncTemperature(0);
    
    // Set the heater state appropriately
    if (lastTemp < targetTemp-1) {
      heaterOn();
    } else {
      heaterOff();
    }
}

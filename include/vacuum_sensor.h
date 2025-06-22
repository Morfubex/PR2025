#ifndef VACUUM_SENSOR_H
#define VACUUM_SENSOR_H

#include <Arduino.h>

const float Vcc = 5.0;
const float Vmin = 0.5;
const float Vmax = 4.5;

struct PressureData {
  float pressure_Pa;
  float pressure_kPa;
  float pressure_MPa;
};

PressureData readVacuumSensor(uint8_t pin) {
  int raw = analogRead(pin);
  float voltage = raw * Vcc / 1023.0;
  voltage = constrain(voltage, Vmin, Vmax);

  PressureData data;
  data.pressure_kPa = ((voltage - Vmin) / (Vmax - Vmin)) * 100.0 - 100.0;
  data.pressure_Pa = data.pressure_kPa * 1000.0;
  data.pressure_MPa = data.pressure_kPa / 1000.0;
  return data;
}

#endif
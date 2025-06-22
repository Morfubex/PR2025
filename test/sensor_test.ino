#include "Arduino.h"
#include "vacuum_sensor.h"

void setup() {
  Serial.begin(9600);
}

void loop() {
  PressureData result = readVacuumSensor(A3);

  static unsigned long lastPrintTime = 0;
  if (millis() - lastPrintTime > 1000) {
    lastPrintTime = millis();
    Serial.print("Pressure: ");
    Serial.print(result.pressure_Pa, 1);
    Serial.print(" Pa | ");
    Serial.print(result.pressure_kPa, 2);
    Serial.print(" kPa | ");
    Serial.print(result.pressure_MPa, 3);
    Serial.println(" MPa");
  }
}
#include <Arduino.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "vacuum_sensor.h"

#define VACUUM_SENSOR_PIN A3

LiquidCrystal_I2C lcd(0x27, 16, 2);  

void setup() {
  lcd.init(); 
  lcd.backlight(); 
}

void loop() {
    PressureData data = readVacuumSensor(VACUUM_SENSOR_PIN);

    static unsigned long lastPrintTime = 0;
    if (millis() - lastPrintTime > 1000) {
        lastPrintTime = millis();
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Pressure:");
        lcd.setCursor(0, 1);
        lcd.print("kPa:");
        lcd.print(data.pressure_kPa, 1);
    }
}
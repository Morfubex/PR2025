#include <Arduino.h>
#include "GyverOLED.h"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "vacuum_sensor.h"

#include "motor.h"
#include "button.h"


Motor motor;

#define VACUUM_SENSOR_PIN A3
#define BUTTON_PIN 8
#define VALVE_PIN 4

GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled; 

void setup() {
  motor.begin();
  Serial.begin(9600);
  oled.init();
  oled.clear();
  oled.setScale(2);
  oled.home();
  oled.print("Pressure");
  digitalWrite(VALVE_PIN, HIGH);
}

void loop() {
    PressureData data = readVacuumSensor(VACUUM_SENSOR_PIN);

    motor.update();
  
    static unsigned long lastChangeTime = 0;
    static uint8_t state = 0;
    
    if (millis() - lastChangeTime > 3000) {
      lastChangeTime = millis();
      state = (state + 1) % 4;
      
      switch(state) {
        case 0: motor.forward(); break;
        case 1: motor.setSpeed(30); break;
        case 2: motor.backward(); break;
        case 3: motor.stop(); break;
      }
    }
    

    static unsigned long lastPrintTime = 0;
    if (millis() - lastPrintTime > 500) {
        lastPrintTime = millis();
        oled.setCursor(0, 3);
        oled.print(data.pressure_kPa, 2);
        oled.setCursor(0, 5);
        oled.print(motor.getSpeed());
        oled.print("  ");
        Serial.print("Current speed: ");
        Serial.println(motor.getSpeed());
    }
}
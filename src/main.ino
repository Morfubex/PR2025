#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "button.h"
#include "motor.h"
#include "servo_control.h"
#include "vacuum_sensor.h"

#define BUTTON_PIN 8
#define LED_PIN 7
#define VALVE_PIN 4
#define VACUUM_SENSOR_PIN A3

Motor mixer;
Servo servo1;
Servo servo2;
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long cycleStartTime = 0;
bool cycleRunning = false;
int currentStep = 0;
unsigned long stepStartTime = 0;
bool valveState = false;

void displayStatus(const String& action, float pressure_kPa, unsigned long elapsedTimeSec) {
  static String lastAction = "";
  static float lastPressure = -999.0;
  static unsigned long lastElapsedTime = -1;
  static unsigned long lastPrintTime = 0;

  if (millis() - lastPrintTime > 500) {
    lastPrintTime = millis();

    if (action != lastAction) {
      lcd.setCursor(0, 0);
      lcd.print("                ");  // Очистка строки
      lcd.setCursor(0, 0);
      lcd.print(action);
      lastAction = action;
    }

    if (abs(pressure_kPa - lastPressure) > 0.1 || elapsedTimeSec != lastElapsedTime) {
      lcd.setCursor(0, 1);
      lcd.print("                ");  // Очистка строки
      lcd.setCursor(0, 1);
      lcd.print("P:");
      lcd.print(pressure_kPa, 1);
      lcd.print("kPa T:");
      lcd.print(elapsedTimeSec);
      lcd.print("s");

      lastPressure = pressure_kPa;
      lastElapsedTime = elapsedTimeSec;
    }
  }
}

void setup() {
  lcd.init();
  lcd.backlight();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(VALVE_PIN, OUTPUT);
//   digitalWrite(VALVE_PIN, LOW);

  mixer.begin();
  servo1.attach(5);
  servo2.attach(6);
  servo1.setSpeed(30);
  servo2.setSpeed(30);
  displayStatus("Waiting...", 0.0, 0);
}

void loop() {
  mixer.update();
  servo1.update();
  servo2.update();

  PressureData data = readVacuumSensor(VACUUM_SENSOR_PIN);
  unsigned long totalElapsedSec = (millis() - cycleStartTime) / 1000;

  if (readButtonPress(BUTTON_PIN)) {
    if (cycleRunning) {
        cycleRunning = false;
        mixer.stop();
        digitalWrite(LED_PIN, LOW);
        digitalWrite(VALVE_PIN, LOW);
        currentStep = 0;
    } else {
        cycleRunning = true;
        cycleStartTime = millis();
        stepStartTime = millis();
        currentStep = 0;
    }
    delay(300);
  }

  if (cycleRunning) {
    unsigned long elapsed = millis() - stepStartTime;

    switch (currentStep) {
      case 0: // Step 1
        displayStatus("Cycle Start", data.pressure_kPa, totalElapsedSec);
        digitalWrite(LED_PIN, HIGH);
        mixer.setSpeed(50);
        stepStartTime = millis();
        currentStep++;
        break;

      case 1: // Step 2
        displayStatus("Servo1 to 20", data.pressure_kPa, totalElapsedSec);
        servo1.smoothWrite(20);
        currentStep++;
        break;

      case 2: // Step 3
        displayStatus("Wait Pressure", data.pressure_kPa, totalElapsedSec);
        if (data.pressure_kPa <= -5.0) {
          digitalWrite(LED_PIN, LOW);
          stepStartTime = millis();
          currentStep++;
        }
        break;

      case 3: // Step 4
        displayStatus("Wait 6 min", data.pressure_kPa, totalElapsedSec);
        if (elapsed >= 1UL * 60UL * 1000UL) {
          servo1.smoothWrite(90);
          servo2.smoothWrite(145);
          stepStartTime = millis();
          currentStep++;
        }
        break;

      case 4: // Step 5
        displayStatus("Servo2 Hold", data.pressure_kPa, totalElapsedSec);
        if (elapsed >= 15000) {
          servo2.smoothWrite(90);
          mixer.setSpeed(100);
          stepStartTime = millis();
          currentStep++;
        }
        break;

      case 5: // Step 6
        displayStatus("Mix Loop", data.pressure_kPa, totalElapsedSec);
        if (elapsed <= 45000) {
          if ((elapsed / 1000) % 2 == 0) {
            servo1.smoothWrite(30);
          } else {
            servo1.smoothWrite(90);
          }
        } else {
          mixer.stop();
          stepStartTime = millis();
          currentStep++;
        }
        break;

      case 6: // Step 7
        displayStatus("Vent -80", data.pressure_kPa, totalElapsedSec);
        digitalWrite(VALVE_PIN, HIGH);
        if (data.pressure_kPa <= -5.0) {
          digitalWrite(VALVE_PIN, LOW);
          stepStartTime = millis();
          currentStep++;
        }
        break;

      case 7: // Step 8
        displayStatus("Dump Wait", data.pressure_kPa, totalElapsedSec);
        servo1.smoothWrite(145);
        if (elapsed >= 60000) {
          servo1.smoothWrite(90);
          stepStartTime = millis();
          currentStep++;
        }
        break;

      case 8: // Step 9
        displayStatus("To Atm", data.pressure_kPa, totalElapsedSec);
        digitalWrite(VALVE_PIN, HIGH);
        if (data.pressure_kPa >= 0.0 && elapsed > 30000) {
          digitalWrite(VALVE_PIN, LOW);
          cycleRunning = false;
          currentStep = 0;
          displayStatus("Done", data.pressure_kPa, totalElapsedSec);
        }
        break;
    }
  } else {
    displayStatus("Waiting...", data.pressure_kPa, 0);
  }
}

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
Servo servo1, servo2;
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long cycleStartTime = 0;
bool cycleRunning = false;
int currentStep = 0;
unsigned long stepStartTime = 0;
bool valveState = false;

String currentLine1 = "";
String currentLine2 = "";
unsigned long lastUpdate = 0;

void displayStatus(String action, float pressure_kPa, unsigned long elapsedTimeSec) {
  static unsigned long lastUpdate = 0;
  static String currentLine1 = "";
  static String currentLine2 = "";
  
  if (millis() - lastUpdate < 1000) return;

  String newLine1 = action.substring(0, 16);
  String newLine2 = "P:" + String(pressure_kPa, 1) + "kPa T:" + String(elapsedTimeSec) + "s";
  
  while (newLine1.length() < 16) newLine1 += " ";
  while (newLine2.length() < 16) newLine2 += " ";

  if (newLine1 != currentLine1) {
    lcd.setCursor(0, 0);
    lcd.print(newLine1);
    currentLine1 = newLine1;
  }
  
  if (newLine2 != currentLine2) {
    lcd.setCursor(0, 1);
    lcd.print(newLine2);
    currentLine2 = newLine2;
  }
  
  lastUpdate = millis();
}

void setup() {
  lcd.init(); lcd.backlight(); Wire.setClock(100000);
  Serial.begin(9600);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(VALVE_PIN, OUTPUT);
  digitalWrite(VALVE_PIN, HIGH);

  mixer.begin();
  servo1.attach(5); servo2.attach(6);
  servo1.setSpeed(30); servo2.setSpeed(30);
  displayStatus("Waiting...", 0.0, 0);
}

void loop() {
  mixer.update();
  servo1.update(); servo2.update();

  PressureData data = readVacuumSensor(VACUUM_SENSOR_PIN);

  if (readButtonPress(BUTTON_PIN)) {
    if (cycleRunning) {
        cycleRunning = false;
        mixer.stop();
        digitalWrite(LED_PIN, LOW);
        digitalWrite(VALVE_PIN, HIGH);
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
    unsigned long totalElapsedSec = (millis() - cycleStartTime) / 1000;

    switch (currentStep) {
      case 0: // Step 1
        Serial.println(currentStep);
        displayStatus("Cycle Start", data.pressure_kPa, totalElapsedSec);
        digitalWrite(LED_PIN, HIGH);
        mixer.setSpeed(50);
        stepStartTime = millis();
        currentStep = 10;
        break;

      case 10: // Step 2
        Serial.println(currentStep);
        displayStatus("Servo1 to 20", data.pressure_kPa, totalElapsedSec);
        servo1.smoothWrite(20);
        currentStep = 20;
        break;

      case 20: // Step 3
        Serial.println(currentStep);
        displayStatus("Wait Pressure", data.pressure_kPa, totalElapsedSec);
        if (data.pressure_kPa <= -15.0) {
          digitalWrite(LED_PIN, LOW);
          stepStartTime = millis();
          currentStep = 30;
        }
        break;

      case 30: // Step 4
        Serial.println(currentStep);
        displayStatus("Wait 6 min", data.pressure_kPa, totalElapsedSec);
        if (elapsed >= 1UL * 60UL * 1000UL) {
          servo1.smoothWrite(0);
          servo2.smoothWrite(145);
          stepStartTime = millis();
          currentStep = 40;
        }
        break;

      case 40: // Step 5
        Serial.println(currentStep);
        displayStatus("Servo2 Hold", data.pressure_kPa, totalElapsedSec);
        if (elapsed >= 15000) {
          servo2.smoothWrite(0);
          mixer.setSpeed(100);
          stepStartTime = millis();
          currentStep = 50;
        }
        break;

      case 50: // Step 6
        Serial.println(currentStep);
        displayStatus("Mix Loop", data.pressure_kPa, totalElapsedSec);

        if (elapsed <= 45000) {
          static bool positionFlag = false;
          static unsigned long lastSwitchTime = 0;

          if (millis() - lastSwitchTime >= 2500) {  // каждые 5 секунд
            positionFlag = !positionFlag;
            servo1.smoothWrite(positionFlag ? 0 : 45);
            lastSwitchTime = millis();
          }
        } else {
          mixer.stop();
          stepStartTime = millis();
          currentStep = 60;
        }
        break;

      case 60: // Step 7
        Serial.println(currentStep);
        displayStatus("Vent -80", data.pressure_kPa, totalElapsedSec);
        digitalWrite(VALVE_PIN, LOW);
        if (data.pressure_kPa <= -5.0) {
          digitalWrite(VALVE_PIN, HIGH);
          stepStartTime = millis();
          currentStep = 70;
        }
        break;

      case 70: // Step 8
        Serial.println(currentStep);
        displayStatus("Dump Wait", data.pressure_kPa, totalElapsedSec);
        servo1.smoothWrite(145);
        if (elapsed >= 60000) {
          servo1.smoothWrite(0);
          stepStartTime = millis();
          currentStep = 80;
        }
        break;

      case 80: // Step 9
        Serial.println(currentStep);
        displayStatus("To Atm", data.pressure_kPa, totalElapsedSec);
        digitalWrite(VALVE_PIN, LOW);
        if (data.pressure_kPa >= 0.0 && elapsed > 30000) {
          digitalWrite(VALVE_PIN, HIGH);
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
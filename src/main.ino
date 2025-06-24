#include <Arduino.h>
#include "button.h"
#include "motor.h"
#include "servo_control.h"
#include "vacuum_sensor.h"
#include "GyverOLED.h"

#define BUTTON_PIN 8
#define LED_PIN 7
#define VALVE_PIN 4
#define VACUUM_SENSOR_PIN A3
#define SERVO_A_PIN 5
#define SERVO_B_PIN 6

#define MAX_PRESSURE 50 //target pressure
#define MIN_PRESSURE 0 //atmospheric pressure
#define OP_PRESSURE 30 //operating pressure

#define DEG_TIME 6*60*1000 //degassing time
#define BtoA_TIME 15*1000 //draining from B to A time
#define MIXING_TIME 45*1000
#define CASTING_TIME 60*1000 //draining from A to mold

#define SERVO_A_SPEED 10
#define SERVO_B_SPEED 30
#define MAX_MIXER_SPEED 100

Motor mixer;
Servo servoA, servoB;
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled; 

unsigned long cycleStartTime = 0;
bool cycleRunning = false;
unsigned long currentStep = 0;
unsigned long stepStartTime = 0;
bool valveState = false;
bool servoflag = 0;
bool valveflag = 0;

String currentLine1 = "";
String currentLine2 = "";
unsigned long lastUpdate = 0;

void displayStatus(String action, float pressure_kPa, unsigned long elapsedTimeSec) {
  static unsigned long lastUpdate = 0;
  static String currentLine1 = "";
  static String currentLine2 = "";
  static String currentLine3 = "";
  
  if (millis() - lastUpdate < 100) return;

  String newLine1 = action.substring(0, 16);
  String newLine2 = "P:" + String(pressure_kPa, 1);
  String newLine3 = "T:" + String(elapsedTimeSec) + "s";
  
  // while (newLine1.length() < 16) newLine1 += " ";
  // while (newLine2.length() < 16) newLine2 += " ";
  // while (newLine3.length() < 16) newLine3 += " ";

  if (newLine1 != currentLine1) {
    oled.setCursor(0, 0);
    oled.print(newLine1);
    oled.print("  ");
    currentLine1 = newLine1;
  }
  
  if (newLine2 != currentLine2) {
    oled.setCursor(0, 3);
    oled.print(newLine2);
    oled.print("  ");
    currentLine2 = newLine2;
  }

  if (newLine3 != currentLine3) {
    oled.setCursor(0, 5);
    oled.print(newLine3);
    oled.print("  ");
    currentLine3 = newLine3;
  }
  
  lastUpdate = millis();
}

void setup() {
  oled.init();
  oled.clear();
  oled.setScale(2);
  oled.home();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(VALVE_PIN, OUTPUT);

  mixer.begin();
  servoA.attach(SERVO_A_PIN); servoB.attach(SERVO_B_PIN);
  servoA.setSpeed(SERVO_A_SPEED); servoB.setSpeed(SERVO_B_SPEED);
  displayStatus("Waiting...", 0.0, 0);
}

void loop() {
  mixer.update(); servoA.update(); servoB.update();

  PressureData data = readVacuumSensor(VACUUM_SENSOR_PIN);

  if (readButtonPress(BUTTON_PIN)) {
    if (cycleRunning) {
        cycleRunning = false;
        digitalWrite(LED_PIN, LOW);
        mixer.stop();
        servoA.smoothWrite(0);
        servoB.smoothWrite(0);
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
      case 0:
        displayStatus("Cycle Start", data.pressure_kPa, totalElapsedSec);
        digitalWrite(LED_PIN, HIGH);
        mixer.setSpeed(MAX_MIXER_SPEED/2);
        stepStartTime = millis();
        currentStep = 10;
        break;

      case 10:
        displayStatus("ServoA to 20", data.pressure_kPa, totalElapsedSec);
        servoA.smoothWrite(20);
        currentStep = 20;
        break;

      case 20:
        displayStatus("Wait Pressure", data.pressure_kPa, totalElapsedSec);
        if (data.pressure_kPa <= MAX_PRESSURE) {
          digitalWrite(LED_PIN, LOW);
          stepStartTime = millis();
          currentStep = 30;
        }
        break;

      case 30:
        displayStatus("Degasing", data.pressure_kPa, totalElapsedSec);
        if (elapsed >= DEG_TIME) {
          servoA.smoothWrite(0);
          servoB.smoothWrite(145);
          stepStartTime = millis();
          currentStep = 40;
        }
        break;

      case 40:
        displayStatus("Draining B to A", data.pressure_kPa, totalElapsedSec);
        if (elapsed >= BtoA_TIME) {
          servoB.smoothWrite(0);
          mixer.setSpeed(MAX_MIXER_SPEED);
          stepStartTime = millis();
          currentStep = 50;
        }
        break;

      case 50:
        displayStatus("Mixing", data.pressure_kPa, totalElapsedSec);

        if (elapsed <= MIXING_TIME) {
          static bool positionFlag = false;
          static unsigned long lastSwitchTime = 0;

          if (millis() - lastSwitchTime >= 2500) {
            positionFlag = !positionFlag;
            servoA.smoothWrite(positionFlag ? 0 : 45);
            lastSwitchTime = millis();
          }
        } else {
          mixer.stop();
          stepStartTime = millis();
          currentStep = 60;
        }
        break;

      case 60:
        displayStatus("Pressure to OP", data.pressure_kPa, totalElapsedSec);
        if (valveflag == 0) {
          digitalWrite(VALVE_PIN, HIGH);
          delay(100);
          digitalWrite(VALVE_PIN, LOW);
          valveflag = 1;
        }
        if (data.pressure_kPa <= OP_PRESSURE) {
          digitalWrite(VALVE_PIN, HIGH);
          delay(100);
          digitalWrite(VALVE_PIN, LOW);
          stepStartTime = millis();
          currentStep = 70;
        }
        break;

      case 70:
        displayStatus("Casting", data.pressure_kPa, totalElapsedSec);
        if (servoflag == 0) {
          servoA.smoothWrite(145);
          servoflag = 1;
        }
        if (elapsed >= CASTING_TIME) {
          servoA.smoothWrite(0);
          stepStartTime = millis();
          currentStep = 80;
        }
        break;

      case 80:
        displayStatus("To Atm", data.pressure_kPa, totalElapsedSec);
        if (valveflag == 1) {
          digitalWrite(VALVE_PIN, HIGH);
          delay(100);
          digitalWrite(VALVE_PIN, LOW);
          valveflag = 0;
        }
        if (data.pressure_kPa >= MIN_PRESSURE && elapsed > 30000) {
          digitalWrite(VALVE_PIN, HIGH);
          delay(100);
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
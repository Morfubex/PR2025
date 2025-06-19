#include "Arduino.h"
#include "motor.h"

Motor motor;

void setup() {
  motor.begin();
  Serial.begin(9600);
}

void loop() {
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
  if (millis() - lastPrintTime > 1000) {
    lastPrintTime = millis();
    Serial.print("Current speed: ");
    Serial.println(motor.getSpeed());
  }
}
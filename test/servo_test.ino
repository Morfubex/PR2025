#include "Arduino.h"
#include "servo_control.h"

Servo servo1;
Servo servo2;

void setup() {
    servo1.attach(5);
    servo1.setSpeed(20);
    servo2.attach(6);
    servo2.setSpeed(50);
    Serial.begin(9600);
}

void loop() {
    servo1.update();
    servo2.update();
    
    static unsigned long lastChangeTime = 0;
    static uint8_t state = 0;
    
    if (millis() - lastChangeTime > 10000) {
        lastChangeTime = millis();
        state = (state + 1) % 4;
        
        switch(state) {
        case 0: servo1.smoothWrite(0); break;
        case 1: servo1.smoothWrite(180); break;
        case 2: servo2.smoothWrite(0); break;
        case 3: servo2.smoothWrite(180); break;
        }
    }
    
    static unsigned long lastPrintTime = 0;
    if (millis() - lastPrintTime > 500) {
        lastPrintTime = millis();
        Serial.print("Current angle servo1: ");
        Serial.println(servo1.read());
        Serial.print("Current angle servo2: ");
        Serial.println(servo2.read());
    }
}
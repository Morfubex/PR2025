#ifndef ULTRA_SIMPLE_SERVO_H
#define ULTRA_SIMPLE_SERVO_H

#include <Arduino.h>

#define MIN_ANGLE 0
#define MAX_ANGLE 180
#define MIN_SPEED 5
#define MAX_SPEED 100
#define STEP_ANGLE 1


class UltraSimpleServo {
private:
    byte pin;
    int currentAngle;
    int targetAngle;
    unsigned long lastPulseTime;
    unsigned long lastMoveTime;
    unsigned int stepDelayMs;
    
    void sendPulse() {
        int pulse = map(currentAngle, MIN_ANGLE, MAX_ANGLE, 500, 2500);
        digitalWrite(pin, HIGH);
        delayMicroseconds(pulse);
        digitalWrite(pin, LOW);
        lastPulseTime = millis();
    }

public:
    UltraSimpleServo() : pin(255), currentAngle(90), targetAngle(90), 
                        lastPulseTime(0), lastMoveTime(0), stepDelayMs(20) {}

    void attach(byte pinNumber) {
        pin = pinNumber;
        pinMode(pin, OUTPUT);
        currentAngle = 90;
        targetAngle = 90;
        sendPulse();
    }

    void write(int angle) {
        angle = constrain(angle, MIN_ANGLE, MAX_ANGLE);
        currentAngle = angle;
        targetAngle = angle;
        if (millis() - lastPulseTime >= 20) {
            sendPulse();
        }
    }

    void smoothWrite(int angle) {
        targetAngle = constrain(angle, MIN_ANGLE, MAX_ANGLE);
        lastMoveTime = millis();
    }

    void setSpeed(float degreesPerSecond) { //anglespeed
        degreesPerSecond = constrain(degreesPerSecond, MIN_SPEED, MAX_SPEED);
        stepDelayMs = 1000.0f / degreesPerSecond;
    }

    void update() {
        if (millis() - lastMoveTime < stepDelayMs) return;
        
        if (currentAngle != targetAngle) {
            currentAngle += (currentAngle < targetAngle) ? STEP_ANGLE : -STEP_ANGLE;
            sendPulse();
            lastMoveTime = millis();
        }
    }

    int read() {
        return currentAngle;
    }
};
#endif
#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class SimpleButton {
private:
    byte buttonPin;
    byte actionPin;
    bool lastState;
    bool currentState;
    bool toggledState;
    unsigned long lastDebounceTime;
    const unsigned long debounceDelay = 50;

public:
    SimpleButton(byte btnPin, byte actPin)
        : buttonPin(btnPin), actionPin(actPin),
          lastState(HIGH), currentState(HIGH),
          toggledState(false), lastDebounceTime(0) {}

    void begin() {
        pinMode(buttonPin, INPUT_PULLUP); //button from GND to PIN
        pinMode(actionPin, OUTPUT);
        digitalWrite(actionPin, LOW);
    }

    void update() {
        bool reading = digitalRead(buttonPin);
        
        if (reading != currentState) {
            lastDebounceTime = millis();
            currentState = reading;
        }

        if ((millis() - lastDebounceTime) > debounceDelay) {
            if (currentState != lastState) {
                lastState = currentState;
                
                if (lastState == LOW) {
                    toggledState = !toggledState;
                    digitalWrite(actionPin, toggledState ? HIGH : LOW);
                }
            }
        }
    }
};

#endif
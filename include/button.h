#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

const unsigned long debounceDelay = 50;

bool readButtonPress(uint8_t pin) {
    static bool lastState = HIGH;
    static unsigned long lastDebounceTime = 0;
    bool currentState = digitalRead(pin);

    if (currentState != lastState) {
        lastDebounceTime = millis();
        lastState = currentState;
    }

    if ((millis() - lastDebounceTime) > debounceDelay && currentState == LOW) {
        // Ждём отпускания кнопки, чтобы не вызывать повторно
        while (digitalRead(pin) == LOW);
        delay(10); // Защита от повторного дребезга
        return true;
    }

    return false;
}

#endif
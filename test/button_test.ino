#include <Arduino.h>
#include "button.h"

#define BUTTON_PIN 8
#define LED_PIN 7

bool blinking = false;
unsigned long lastBlinkTime = 0;
bool ledState = LOW;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  if (readButtonPress(BUTTON_PIN)) {
    blinking = !blinking;
    if (!blinking) {
      digitalWrite(LED_PIN, LOW); 
    }
  }

  if (blinking && millis() - lastBlinkTime >= 500) {
    lastBlinkTime = millis();
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
}
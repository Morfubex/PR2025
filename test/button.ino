#include "button.h"

#define BUTTON_PIN 8
#define LED_PIN 7

SimpleButton button(BUTTON_PIN, LED_PIN);

void setup() {
  button.begin();
}

void loop() {
  button.update();
}
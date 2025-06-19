#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

#define MOTOR_IN1_PIN 11
#define MOTOR_IN2_PIN 12
#define PWM_RESOLUTION 255
#define RAMP_INTERVAL_MOTOR 10 //change interval (ms)

class Motor {
private:
  int8_t currentSpeed = 0;
  int8_t targetSpeed = 0;
  unsigned long prevUpdateTime = 0;

  void applySpeed(int8_t speed) {
    if (speed > 0) {
      analogWrite(MOTOR_IN1_PIN, map(speed, 0, 100, 0, PWM_RESOLUTION));
      digitalWrite(MOTOR_IN2_PIN, LOW);
    } 
    else if (speed < 0) {
      digitalWrite(MOTOR_IN1_PIN, LOW);
      analogWrite(MOTOR_IN2_PIN, map(-speed, 0, 100, 0, PWM_RESOLUTION));
    } 
    else {
      digitalWrite(MOTOR_IN1_PIN, LOW);
      digitalWrite(MOTOR_IN2_PIN, LOW);
    }
  }

public:
  void begin() {
    pinMode(MOTOR_IN1_PIN, OUTPUT);
    pinMode(MOTOR_IN2_PIN, OUTPUT);
    stop();
  }

  void setSpeed(int8_t speed) {
    targetSpeed = constrain(speed, -100, 100);
  }

  void forward() {
    setSpeed(100);
  }

  void backward() {
    setSpeed(-100);
  }

  void stop() {
    setSpeed(0);
  }

  void emergencyStop() {
    currentSpeed = 0;
    targetSpeed = 0;
    applySpeed(0);
  }

  void update() {
    unsigned long currentTime = millis();
    
    if (currentTime - prevUpdateTime >= RAMP_INTERVAL_MOTOR) {
      if (currentSpeed < targetSpeed) {
        currentSpeed++;
        applySpeed(currentSpeed);
      } 
      else if (currentSpeed > targetSpeed) {
        currentSpeed--;
        applySpeed(currentSpeed);
      }
      prevUpdateTime = currentTime;
    }
  }

  int8_t getSpeed() {
    return currentSpeed;
  }
};
#endif
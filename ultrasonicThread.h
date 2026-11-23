#ifndef ULTRASONICTHREAD_H
#define ULTRASONICTHREAD_H

#include <Arduino.h>
#include <Thread.h>
#include "config.h"

class UltrasonicThread : public Thread {
  private:
    int _trigPin;
    int _echoPin;

  public:
    UltrasonicThread(int trigPin, int echoPin, unsigned long interval) {
      _trigPin = trigPin;
      _echoPin = echoPin;
      setInterval(interval);
    }

    void run() {
      digitalWrite(_trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(_trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(_trigPin, LOW);

      long duration = pulseIn(_echoPin, HIGH, 25000);

      if (duration > 0) {
        state.distance = (duration * 0.034) / 2;
      } else {
        state.distance = 999;
      }

      runned();
    }
};

#endif

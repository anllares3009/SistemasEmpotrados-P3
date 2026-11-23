#ifndef LEDTHREAD_H
#define LEDTHREAD_H

#include <Arduino.h>
#include <Thread.h>
#include "config.h"

class LedThread : public Thread {
  private:
    int _pinLed1;
    int _pinLed2;
    int _tickCounter;

  public:
    LedThread(int pinLed1, int pinLed2, unsigned long interval) {
      _pinLed1 = pinLed1;
      _pinLed2 = pinLed2;
      _tickCounter = 0;
      setInterval(interval);
    }

    void run() {
      if (state.actual_state == MODO_ARRANQUE) {
        analogWrite(_pinLed2, 0);
        _tickCounter++;

        if (_tickCounter >= 10) {
          _tickCounter = 0;
          state.led_start_state = !state.led_start_state;
          digitalWrite(_pinLed1, state.led_start_state ? HIGH : LOW);

          if (!state.led_start_state) {
            state.blink_cycles++;
            if (state.blink_cycles >= 3) {
              state.actual_state = MODO_SERVICIO;
              state.substate_service = SERV_ESPERANDO_CLIENTE;
              state.forceLCD = true;
            }
          }
        }
      } 
      else if (state.actual_state == MODO_SERVICIO && state.substate_service == SERV_PREPARANDO_BEBIDA) {
        digitalWrite(_pinLed1, LOW);
        unsigned long elapsed = millis() - state.state_start_time;
        int brightness = map(elapsed, 0, state.prep_time, 0, 255);
        brightness = constrain(brightness, 0, 255);
        analogWrite(_pinLed2, brightness);
      } 
      else if (state.actual_state == MODO_ADMIN) {
        digitalWrite(_pinLed1, HIGH);
        analogWrite(_pinLed2, 255);
        _tickCounter = 0;
      }
      else {
        digitalWrite(_pinLed1, LOW);
        analogWrite(_pinLed2, 0);
        _tickCounter = 0;
      }

      runned();
    }
};

#endif
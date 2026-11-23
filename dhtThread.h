#ifndef DHTTHREAD_H
#define DHTTHREAD_H

#include <Arduino.h>
#include <Thread.h>
#include <DHT.h>
#include "config.h"

class DhtThread : public Thread {
  private:
    DHT _dht;

  public:
    DhtThread(int dhtPin, unsigned long interval) : _dht(dhtPin, DHT11) {
      setInterval(interval);
    }

    void begin() {
      _dht.begin();
    }
  
    void run() {
      float h = _dht.readHumidity();
      float t = _dht.readTemperature();

      if (!isnan(h) && !isnan(t)) {
      state.humidity = h;
      state.temperature = t;
      }
      runned();
    }
};

#endif

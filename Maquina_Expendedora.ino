#include <Arduino.h>
#include <avr/wdt.h>
#include <Thread.h>
#include <ThreadController.h>

#include "config.h"
#include "ultrasonicThread.h"
#include "dhtThread.h"
#include "joystickThread.h"
#include "ledThread.h"
#include "lcdThread.h"

GlobalState state;

Product catalog[PRODUCTS] = {
  {"Cafe Solo",     1.00},
  {"Cafe Cortado",  1.10},
  {"Cafe Doble",    1.25},
  {"Cafe Premium",  1.50},
  {"Chocolate",     2.00}
};

ThreadController controller = ThreadController();

UltrasonicThread ultrasonicThread(PIN_TRIG, PIN_ECHO, 200);
DhtThread dhtThread(PIN_DHT, 2000);
JoystickThread joystickThread(PIN_JS_X, PIN_JS_Y, PIN_JS_DATA, 100);
LedThread ledThread(PIN_LED1, PIN_LED2, 100);
LcdThread lcdThread(PIN_LCD_RS, PIN_LCD_E, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7, 100);

volatile unsigned long last_edge_time = 0;
volatile unsigned long last_duration = 0;   // solo para depuracion por Serial
volatile bool new_duration = false;

void isrBotonExterno() {
  unsigned long ahora = millis();
  if (ahora - last_edge_time < 30) return;   // antirrebote
  last_edge_time = ahora;

  bool estadoPin = digitalRead(PIN_BUTTON);

  if (estadoPin == LOW) {
    pressed_button = true;
    start_pulse_time = millis();
  } else {
    if (pressed_button) {
      unsigned long duracion = millis() - start_pulse_time;
      pressed_button = false;
      last_duration = duracion;
      new_duration = true;

      if (duracion >= 2000 && duracion <= 3000) {
        restart_service = true;
      } else if (duracion >= 5000) {
        start_admin = true;
      }
    }
  }
}

void procesarEventosInterrupcion() {
  if (new_duration) {
    new_duration = false;
    Serial.print("Pulsacion: ");
    Serial.print(last_duration);
    Serial.print(" ms, modo=");
    Serial.println(state.actual_state);   // 0=arranque 1=servicio 2=admin
  }
  if (restart_service) {
    restart_service = false;
    if (state.actual_state == MODO_SERVICIO) {
      state.substate_service = SERV_ESPERANDO_CLIENTE;
      state.product_index = 0;
      state.forceLCD = true;
    }
  }

  if (start_admin) {
    start_admin = false;
    if (state.actual_state == MODO_ADMIN) {
      state.actual_state = MODO_SERVICIO;
      state.substate_service = SERV_ESPERANDO_CLIENTE;
      state.product_index = 0;
    } else if (state.actual_state == MODO_SERVICIO) {
      state.actual_state = MODO_ADMIN;
      state.substate_admin = ADMIN_MENU_PRINCIPAL;
      state.admin_menu_index = 0;
    }
    state.forceLCD = true;
  }
}

void setup() {
  wdt_disable();
  Serial.begin(9600);

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_JS_DATA, INPUT_PULLUP);

  randomSeed(analogRead(A3));

  dhtThread.begin();
  lcdThread.begin();

  controller.add(&ultrasonicThread);
  controller.add(&dhtThread);
  controller.add(&joystickThread);
  controller.add(&ledThread);
  controller.add(&lcdThread);

  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON), isrBotonExterno, CHANGE);

  wdt_enable(WDTO_4S);
}

void loop() {
  wdt_reset();
  procesarEventosInterrupcion();
  controller.run();
}
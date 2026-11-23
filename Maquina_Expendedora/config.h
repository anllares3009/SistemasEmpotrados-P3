#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

const int PIN_LCD_RS = 12;
const int PIN_LCD_E  = 11;
const int PIN_LCD_D4 = 6;
const int PIN_LCD_D5 = 5;
const int PIN_LCD_D6 = 4;
const int PIN_LCD_D7 = 3;

const int PIN_BUTTON    = 2;
const int PIN_LED1      = 10;
const int PIN_LED2      = 9;  // PWM
const int PIN_TRIG      = 8;  
const int PIN_ECHO      = 7;
const int PIN_DHT       = A2;
const int PIN_JS_X      = A0;
const int PIN_JS_Y      = A1;
const int PIN_JS_DATA   = 13;

enum SystemState {
  MODO_ARRANQUE,
  MODO_SERVICIO,
  MODO_ADMIN
};

enum ServiceSubstate {
  SERV_ESPERANDO_CLIENTE,
  SERV_MOSTRAR_AMBIENTE,
  SERV_SELECCION_PRODUCTO,
  SERV_PREPARANDO_BEBIDA,
  SERV_RETIRE_BEBIDA
};

enum AdminSubstate {
  ADMIN_MENU_PRINCIPAL,
  ADMIN_VER_TEMP,
  ADMIN_VER_DIST,
  ADMIN_VER_CONTADOR,
  ADMIN_MOD_PRECIOS_LISTA,
  ADMIN_MOD_PRECIOS_EDITAR
};

volatile bool pressed_button = false;
volatile unsigned long start_pulse_time = 0;
volatile bool restart_service = false;        // Pulsacion entre 2s y 3s
volatile bool start_admin = false;            // Pulsacion >= 5s

struct GlobalState {
  SystemState actual_state = MODO_ARRANQUE;
  ServiceSubstate substate_service = SERV_ESPERANDO_CLIENTE;
  AdminSubstate substate_admin = ADMIN_MENU_PRINCIPAL;

  int blink_cycles = 0;
  bool led_start_state = false;

  float temperature = 0.0;
  float humidity = 0.0;
  long distance = 999;

  unsigned long state_start_time = 0;
  unsigned long prep_time = 5000;

  int product_index = 0;
  int admin_menu_index = 0;
  float temp_prize = 0.0;

  bool forceLCD = true;
};

extern GlobalState state;

struct Product {
  const char* name;
  float prize;
};

const int PRODUCTS = 5;
extern Product catalog[PRODUCTS];

inline void actualizarPrecio(int index, float newPrize) {
  if (index >= 0 && index < PRODUCTS) {
    catalog[index].prize = newPrize;
    if(newPrize < 0.05) catalog[index].prize = 0.05;
  }
}

#endif
#ifndef LCDTHREAD_H
#define LCDTHREAD_H

#include <Arduino.h>
#include <Thread.h>
#include <LiquidCrystal.h>
#include "config.h"

class LcdThread : public Thread {
  private:
    LiquidCrystal _lcd;
    unsigned long _lastSensorRefresh;

  public:
    LcdThread(int rs, int e, int d4, int d5, int d6, int d7, unsigned long interval)
      : _lcd(rs, e, d4, d5, d6, d7) {
      _lastSensorRefresh = 0;
      setInterval(interval);
    }

    void begin() {
      _lcd.begin(16, 2);
      _lcd.clear();
    }

    void run() {
      actualizarTransicionesTiempo();

      bool isLiveSensorView = (state.actual_state == MODO_ADMIN &&
                              (state.substate_admin == ADMIN_VER_TEMP ||
                               state.substate_admin == ADMIN_VER_DIST ||
                               state.substate_admin == ADMIN_VER_CONTADOR));

      if (state.forceLCD || (isLiveSensorView && millis() - _lastSensorRefresh >= 1000)) {
        state.forceLCD = false;
        _lastSensorRefresh = millis();
        dibujarPantalla();
      }

      runned();
    }

  private:
    void actualizarTransicionesTiempo() {
      if (state.actual_state == MODO_SERVICIO) {
        if (state.substate_service == SERV_ESPERANDO_CLIENTE) {
          if (state.distance < 100) {
            state.substate_service = SERV_MOSTRAR_AMBIENTE;
            state.state_start_time = millis();
            state.forceLCD = true;
          }
        } 
        else if (state.substate_service == SERV_MOSTRAR_AMBIENTE) {
          if (millis() - state.state_start_time >= 5000) {
            state.substate_service = SERV_SELECCION_PRODUCTO;
            state.product_index = 0;
            state.forceLCD = true;
          }
        } 
        else if (state.substate_service == SERV_PREPARANDO_BEBIDA) {
          if (millis() - state.state_start_time >= state.prep_time) {
            state.substate_service = SERV_RETIRE_BEBIDA;
            state.state_start_time = millis();
            state.forceLCD = true;
          }
        } 
        else if (state.substate_service == SERV_RETIRE_BEBIDA) {
          if (millis() - state.state_start_time >= 3000) {
            state.substate_service = SERV_ESPERANDO_CLIENTE;
            state.forceLCD = true;
          }
        }
      }
    }

    void dibujarPantalla() {
      _lcd.clear();
      _lcd.setCursor(0, 0);

      if (state.actual_state == MODO_ARRANQUE) {
        _lcd.print("CARGANDO...");
      } 
      else if (state.actual_state == MODO_SERVICIO) {
        switch (state.substate_service) {
          case SERV_ESPERANDO_CLIENTE:
            _lcd.print("ESPERANDO");
            _lcd.setCursor(0, 1);
            _lcd.print("CLIENTE");
            break;

          case SERV_MOSTRAR_AMBIENTE:
            _lcd.print("Temp: ");
            _lcd.print(state.temperature, 1);
            _lcd.print(" C");
            _lcd.setCursor(0, 1);
            _lcd.print("Hum:  ");
            _lcd.print(state.humidity, 1);
            _lcd.print(" %");
            break;

          case SERV_SELECCION_PRODUCTO:
            _lcd.print(catalog[state.product_index].name);
            _lcd.setCursor(0, 1);
            _lcd.print("Precio: ");
            _lcd.print(catalog[state.product_index].prize, 2);
            _lcd.print(" E");
            break;

          case SERV_PREPARANDO_BEBIDA:
            _lcd.print("Preparando");
            _lcd.setCursor(0, 1);
            _lcd.print("Cafe...");
            break;

          case SERV_RETIRE_BEBIDA:
            _lcd.print("RETIRE BEBIDA");
            break;
        }
      } 
      else if (state.actual_state == MODO_ADMIN) {
        switch (state.substate_admin) {
          case ADMIN_MENU_PRINCIPAL:
            _lcd.print("> ");
            if (state.admin_menu_index == 0) _lcd.print("Temperatura");
            else if (state.admin_menu_index == 1) _lcd.print("Distancia");
            else if (state.admin_menu_index == 2) _lcd.print("Contador");
            else if (state.admin_menu_index == 3) _lcd.print("Precios");
            break;

          case ADMIN_VER_TEMP:
            _lcd.print("T:");
            _lcd.print(state.temperature, 1);
            _lcd.print("C H:");
            _lcd.print(state.humidity, 1);
            _lcd.print("%");
            break;

          case ADMIN_VER_DIST:
            _lcd.print("Distancia:");
            _lcd.setCursor(0, 1);
            _lcd.print(state.distance);
            _lcd.print(" cm");
            break;

          case ADMIN_VER_CONTADOR:
            _lcd.print("Tiempo activo:");
            _lcd.setCursor(0, 1);
            _lcd.print(millis() / 1000);
            _lcd.print(" s");
            break;

          case ADMIN_MOD_PRECIOS_LISTA:
            _lcd.print(catalog[state.product_index].name);
            _lcd.setCursor(0, 1);
            _lcd.print(catalog[state.product_index].prize, 2);
            _lcd.print(" E (Click)");
            break;

          case ADMIN_MOD_PRECIOS_EDITAR:
            _lcd.print("Nuevo precio:");
            _lcd.setCursor(0, 1);
            _lcd.print(state.temp_prize, 2);
            _lcd.print(" E");
            break;
        }
      }
    }
};

#endif
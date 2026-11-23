#ifndef JOYSTICKTHREAD_H
#define JOYSTICKTHREAD_H

#include <Arduino.h>
#include <Thread.h>
#include "config.h"

class JoystickThread : public Thread {
  private:
    int _pinX;
    int _pinY;
    int _pinBtn;
    bool _isCentered;
    bool _lastBtnState;

  public:
    JoystickThread(int pinX, int pinY, int pinBtn, unsigned long interval) {
      _pinX = pinX;
      _pinY = pinY;
      _pinBtn = pinBtn;
      _isCentered = true;
      _lastBtnState = HIGH;
      setInterval(interval);
    }

    void run() {
      int y = analogRead(_pinY);
      int x = analogRead(_pinX);
      bool btnState = digitalRead(_pinBtn);

      if (y < 300 && _isCentered) {
        _isCentered = false;
        manejarArriba();
      } else if (y > 700 && _isCentered) {
        _isCentered = false;
        manejarAbajo();
      } else if (y >= 300 && y <= 700 && x >= 300 && x <= 700) {
        _isCentered = true;
      }

      if (x < 300 && _isCentered) {
        _isCentered = false;
        manejarIzquierda();
      }

      if (btnState == LOW && _lastBtnState == HIGH) {
        manejarClick();
      }
      _lastBtnState = btnState;

      runned();
    }

  private:
    void manejarArriba() {
      if (state.actual_state == MODO_SERVICIO && state.substate_service == SERV_SELECCION_PRODUCTO) {
        state.product_index = (state.product_index - 1 + PRODUCTS) % PRODUCTS;
        state.forceLCD = true;
      } else if (state.actual_state == MODO_ADMIN) {
        if (state.substate_admin == ADMIN_MENU_PRINCIPAL) {
          state.admin_menu_index = (state.admin_menu_index - 1 + 4) % 4;
          state.forceLCD = true;
        } else if (state.substate_admin == ADMIN_MOD_PRECIOS_LISTA) {
          state.product_index = (state.product_index - 1 + PRODUCTS) % PRODUCTS;
          state.forceLCD = true;
        } else if (state.substate_admin == ADMIN_MOD_PRECIOS_EDITAR) {
          state.temp_prize += 0.05;
          state.forceLCD = true;
        }
      }
    }

    void manejarAbajo() {
      if (state.actual_state == MODO_SERVICIO && state.substate_service == SERV_SELECCION_PRODUCTO) {
        state.product_index = (state.product_index + 1) % PRODUCTS;
        state.forceLCD = true;
      } else if (state.actual_state == MODO_ADMIN) {
        if (state.substate_admin == ADMIN_MENU_PRINCIPAL) {
          state.admin_menu_index = (state.admin_menu_index + 1) % 4;
          state.forceLCD = true;
        } else if (state.substate_admin == ADMIN_MOD_PRECIOS_LISTA) {
          state.product_index = (state.product_index + 1) % PRODUCTS;
          state.forceLCD = true;
        } else if (state.substate_admin == ADMIN_MOD_PRECIOS_EDITAR) {
          state.temp_prize -= 0.05;
          if (state.temp_prize < 0.05) state.temp_prize = 0.05;
          state.forceLCD = true;
        }
      }
    }

    void manejarIzquierda() {
      if (state.actual_state != MODO_ADMIN) return;

      if (state.substate_admin == ADMIN_MOD_PRECIOS_EDITAR) {
        // Descartar cambios y volver al listado
        state.substate_admin = ADMIN_MOD_PRECIOS_LISTA;
        state.forceLCD = true;
      } else if (state.substate_admin != ADMIN_MENU_PRINCIPAL) {
        state.substate_admin = ADMIN_MENU_PRINCIPAL;
        state.forceLCD = true;
      }
    }

    void manejarClick() {
      if (state.actual_state == MODO_SERVICIO && state.substate_service == SERV_SELECCION_PRODUCTO) {
        state.substate_service = SERV_PREPARANDO_BEBIDA;
        state.state_start_time = millis();
        state.prep_time = random(4000, 8001); // Entre 4 y 8 segundos aleatorios
        state.forceLCD = true;
      } 
      else if (state.actual_state == MODO_ADMIN) {
        if (state.substate_admin == ADMIN_MENU_PRINCIPAL) {
          if (state.admin_menu_index == 0) state.substate_admin = ADMIN_VER_TEMP;
          else if (state.admin_menu_index == 1) state.substate_admin = ADMIN_VER_DIST;
          else if (state.admin_menu_index == 2) state.substate_admin = ADMIN_VER_CONTADOR;
          else if (state.admin_menu_index == 3) state.substate_admin = ADMIN_MOD_PRECIOS_LISTA;
          state.forceLCD = true;
        } 
        else if (state.substate_admin == ADMIN_MOD_PRECIOS_LISTA) {
          state.temp_prize = catalog[state.product_index].prize;
          state.substate_admin = ADMIN_MOD_PRECIOS_EDITAR;
          state.forceLCD = true;
        } 
        else if (state.substate_admin == ADMIN_MOD_PRECIOS_EDITAR) {
          actualizarPrecio(state.product_index, state.temp_prize);
          state.substate_admin = ADMIN_MOD_PRECIOS_LISTA;
          state.forceLCD = true;
        }
      }
    }
};

#endif
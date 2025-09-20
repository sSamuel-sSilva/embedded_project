#ifndef LED_H
#define LED_H


#include <stdlib.h>
#include <stdint.h>


enum COLORS {
    BRANCO,
    AMARELO,
    VERMELHO,
    VERDE,
    AZUL,
    MAGENTA,
    CIANO,
    DESLIGADO
};


// extern uint8_t led_state;
// extern uint32_t last_led_change;


void init_led(int led);
void led_set_color(uint8_t color);

#endif
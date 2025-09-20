#include "../include/led.h"
#include "pico/stdlib.h"
#include <stdlib.h>
#include <stdint.h>


// uint8_t led_state = DESLIGADO;
// uint32_t last_led_change = 0;


uint8_t colors[8][3] = {
    {1, 1, 1}, // Branco  
    {1, 0, 1}, // Amarelo 
    {0, 0, 1}, // Vermelho
    {1, 0, 0}, // Verde   
    {0, 1, 0}, // Azul    
    {0, 1, 1}, // Magenta  
    {1, 1, 0}, // Ciano    
    {0, 0, 0}  // Desligado
};


void init_led(int led)
{
    gpio_init(led);
    gpio_set_dir(led, GPIO_OUT);
}


void led_set_color(uint8_t color)
{
    gpio_put(11, colors[color][0]);
    gpio_put(12, colors[color][1]);
    gpio_put(13, colors[color][2]);
}
#include "pico/stdlib.h"
#include "../include/button.h"
#include "../include/led.h"
#include <stdint.h>
#include <stdio.h>

bool flags[2] = {false, false};

button init_button(int button_gpio, uint32_t delay_press, void (*interruption)())
{
    gpio_set_function(button_gpio, GPIO_FUNC_SIO);
    gpio_set_dir(button_gpio, GPIO_IN);
    gpio_pull_up(button_gpio);

    button current;
    current.gpio = button_gpio;
    current.intrr_func = interruption;
    current.last_tick = 0;
    return current;
}


void toggle_read_report_travel()
{
    toggle_flag(FLAG_REPORT_TRAVEL);
}


void toggle_read_report_stop()
{
    toggle_flag(FLAG_REPORT_STOP);
}


void toggle_flag(uint8_t flag_idx)
{
    for (int i = 0; i < 2; i++)
    {
        if (i == flag_idx) flags[i] = !flags[i];
        else flags[i] = false;
    }
}


bool check_flag(uint8_t flag_idx)
{
    return flags[flag_idx];
}
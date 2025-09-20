#ifndef BUTTONS_H
#define BUTTONS_H

typedef struct b {
    uint8_t gpio;
    uint32_t last_tick;
    void(*intrr_func)();
} button;

enum FLAGS_TRAVEL {
    FLAG_REPORT_TRAVEL,
    FLAG_REPORT_STOP
};


button init_button(int button_gpio, uint32_t delay_press, void (*interruption)());

void toggle_read_report_travel();
void toggle_read_report_stop();

void toggle_flag(uint8_t flag_idx);
bool check_flag(uint8_t flag_idx);

#endif
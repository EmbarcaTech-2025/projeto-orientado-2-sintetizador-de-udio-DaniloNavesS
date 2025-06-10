#ifndef DISPLAY_MODULE_H
#define DISPLAY_MODULE_H

#include <stdint.h>

void display_init();
void display_show_message(const char* line1, const char* line2);
void display_show_message_escala1(const char* line1, const char* line2);
void display_draw_waveform(const volatile int16_t* buffer, uint32_t start_index, uint32_t len);
void display_clear();

#endif // DISPLAY_MODULE_H
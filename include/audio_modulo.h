#ifndef AUDIO_MODULE_H
#define AUDIO_MODULE_H

#include "pico/stdlib.h"

// --- Funções Públicas ---

// Inicializa os periféricos de áudio (ADC e PWM)
void audio_init();

// Inicia o processo de gravação
void audio_record();

// Processa o áudio gravado (filtros, ganho, etc.)
void audio_process();

// Reproduz o áudio processado no buzzer
void audio_play();

void audio_stop_playback();
bool audio_is_playing();

const volatile int16_t* audio_get_buffer_ptr();
uint32_t audio_get_playback_index();

#endif // AUDIO_MODULE_H
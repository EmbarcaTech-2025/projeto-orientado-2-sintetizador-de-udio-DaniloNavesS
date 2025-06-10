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

#endif // AUDIO_MODULE_H
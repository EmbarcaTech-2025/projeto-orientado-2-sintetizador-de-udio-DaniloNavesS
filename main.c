#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"

// Nossos próprios módulos
#include "config.h"
#include "audio_modulo.h"
#include "display_modulo.h"

// Função para inicializar o hardware não relacionado ao áudio
void system_init() {
    set_sys_clock_khz(125000, true);
    stdio_init_all();
    sleep_ms(2000); // Espera para o terminal serial conectar
    
    // Watchdog com timeout de 6 segundos
    watchdog_enable(6000, 1);

    // Inicialização do botão
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
}

int main() {
    system_init();
    audio_init();

    // Gravação inicial
    display_show_message_escala1("Pressione B", "para Gravar!");

    uint32_t last_waveform_position = 0;
    const volatile int16_t* audio_buffer = audio_get_buffer_ptr();
    bool gravando = false;

    while (true) {
        watchdog_update();

        // Lógica de iniciar a reprodução
        if (gpio_get(BUTTON_A) == 0 && !audio_is_playing()) {
            audio_play();
            last_waveform_position = 0; 
         }

        if (gpio_get(BUTTON_B) == 0 && !gravando)
        {
            gravando = true;
            audio_record();
            audio_process();
            gravando = false;
            display_show_message_escala1("Pressione A", "para Reproduz!");
        }
        
 
         // Lógica de atualizar o display DURANTE a reprodução (agora corrigida)
         if (audio_is_playing()) {
             // Usa a função getter para pegar o índice atual
             uint32_t current_position = audio_get_playback_index();
 
             if (current_position / 128 > last_waveform_position / 128) {
                  // Usa o ponteiro que pegamos para desenhar
                  display_draw_waveform(audio_buffer, current_position, NUM_SAMPLES - current_position);
                  last_waveform_position = current_position;
             }
         }
     }
     return 0;
 }
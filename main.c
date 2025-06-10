#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"

// Nossos próprios módulos
#include "config.h"
#include "audio_modulo.h"

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

    printf("\nInicio do Programa\n");

    // Etapa 2: Loop de Reprodução
    bool reproduzindo = false;
    bool gravando = false;
    while (true) {
        if (gpio_get(BUTTON_A) == 0 && !reproduzindo && !reproduzindo) {
            reproduzindo = true;
            audio_play();
            reproduzindo = false;
        }

        if (gpio_get(BUTTON_B) == 0 && !gravando && !reproduzindo)
        {
            gravando = true;
            audio_record();
            audio_process();
            gravando = false;
        }

        watchdog_update();
        sleep_ms(50); // Pequeno delay para evitar debounce e consumo excessivo
    }

    return 0; // Nunca será alcançado
}
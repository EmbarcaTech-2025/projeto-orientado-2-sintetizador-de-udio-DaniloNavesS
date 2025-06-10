#include "audio_modulo.h"
#include "config.h" // Inclui nossas configurações
#include "display_modulo.h"

// Headers do SDK necessários para este módulo
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include <stdio.h>

// --- Variáveis Estáticas (privadas ao módulo) ---
static volatile uint16_t raw_buffer[NUM_SAMPLES];
static volatile int16_t processed_buffer[NUM_SAMPLES];
static volatile uint32_t index_gravacao = 0;
static int16_t dc_offset = 0; // Armazena o offset DC calculado
static repeating_timer_t timer;

// --- Funções Privadas (Callbacks e Auxiliares) ---
static bool amostragem_callback(repeating_timer_t *t) {
    if (index_gravacao < NUM_SAMPLES) {
        raw_buffer[index_gravacao++] = adc_read();
        return true; // Continua o timer
    }
    return false; // Para o timer
}

// --- Implementação das Funções Públicas ---
void audio_init() {
    // Inicialização Microfone
    adc_init();
    adc_gpio_init(MIC_PIN);
    adc_select_input(CANAL_ADC);
    // Inicialização dos LEDS
    gpio_init(LED_GRAVACAO_PIN);
    gpio_set_dir(LED_GRAVACAO_PIN, GPIO_OUT);

    gpio_init(LED_REPRODUCAO_PIN);
    gpio_set_dir(LED_REPRODUCAO_PIN, GPIO_OUT);

    display_init();
    display_show_message_escala1("Pressione B", "para Gravar!");

    // Inicialização Buzzer
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 3.815f); // Ajuste para ~32.7kHz PWM freq
    pwm_config_set_wrap(&config, 1023);     // Resolução de 10 bits
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(BUZZER_PIN, 0);
}

void audio_record() {
    display_show_message("Gravando", "...");
    printf("Gravando...\n");
    gpio_put(LED_GRAVACAO_PIN, 1);
    index_gravacao = 0; // Reseta o índice para uma nova gravação
    add_repeating_timer_us(-SAMPLE_INTERVAL_US, amostragem_callback, NULL, &timer);

    while (index_gravacao < NUM_SAMPLES) {
        watchdog_update();
        tight_loop_contents(); // Otimização do Pico SDK para loops de espera
    }
    cancel_repeating_timer(&timer);
    printf("Gravação encerrada.\n");
    display_show_message_escala1("Pressione A", " para Reproduz!");
    gpio_put(LED_GRAVACAO_PIN, 0);
}

void audio_process() {
    printf("Aplicando filtros e ganho...\n");
    
    // 1. Cálculo e remoção do Offset DC
    long long sum = 0;
    for (uint32_t i = 0; i < NUM_SAMPLES; i++) sum += raw_buffer[i];
    dc_offset = sum / NUM_SAMPLES;
    
    // 2. Filtro Passa-Alta (High-Pass Filter) para remover ruído de baixa frequência
    processed_buffer[0] = 0;
    for (uint32_t i = 1; i < NUM_SAMPLES; i++) {
        processed_buffer[i] = (int16_t)( (HPF_ALPHA * (raw_buffer[i] - dc_offset)) + ((1.0f - HPF_ALPHA) * processed_buffer[i-1]) );
    }

    // 3. Aplicação de Ganho Digital e Clipping
    for (uint32_t i = 0; i < NUM_SAMPLES; i++) {
        int32_t boosted_sample = (int32_t)(processed_buffer[i] * GAIN_FACTOR);

        if (boosted_sample > CLIP_LIMIT) boosted_sample = CLIP_LIMIT;
        if (boosted_sample < -CLIP_LIMIT) boosted_sample = -CLIP_LIMIT;
        
        processed_buffer[i] = (int16_t)boosted_sample;
    }
    printf("Filtros e ganho aplicados.\n");
}

void audio_play() {
    printf("Reproduzindo...\n");
    gpio_put(LED_REPRODUCAO_PIN, 1);
    for (uint32_t i = 0; i < NUM_SAMPLES; i++) {

        if (i % 128 == 0) {
            display_draw_waveform(processed_buffer, i, NUM_SAMPLES - i);
        }

        // Centraliza a amostra em 512 (metade da resolução de 10 bits)
        int32_t pwm_value = processed_buffer[i] + (CLIP_LIMIT / 2); 
        
        // Converte para a escala de 10 bits do PWM
        uint16_t pwm_value_10bit = pwm_value >> 2; 
        if (pwm_value_10bit > 1023) pwm_value_10bit = 1023;
        
        pwm_set_gpio_level(BUZZER_PIN, pwm_value_10bit);
        sleep_us(SAMPLE_INTERVAL_US);
        watchdog_update();
    }
    pwm_set_gpio_level(BUZZER_PIN, 0); // Desliga o buzzer no final
    printf("Reprodução finalizada.\n");
    display_show_message_escala1("Pressione B", "para Gravar!");
    gpio_put(LED_REPRODUCAO_PIN, 0);

}
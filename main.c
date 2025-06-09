#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"

// Pinagem
#define MIC_PIN 28
#define CANAL_ADC 2
#define BUZZER_PIN 21
#define BUTTON_A 5

#define DURATION_SEC 7
#define SAMPLE_RATE 8000
#define NUM_SAMPLES (SAMPLE_RATE * DURATION_SEC)
#define SAMPLE_INTERVAL_US (1000000 / SAMPLE_RATE)

// Variaveis de estados
bool status_reproducao = false;

// ==========================================================
// FATOR DE GANHO DIGITAL - AJUSTE O VOLUME AQUI!
// ==========================================================
// 1.0 = Sem ganho
// 1.5 = 50% mais volume
// 2.0 = 100% mais volume (dobro)
// Comece com 1.5 e aumente com cuidado para evitar distorção.
const float GAIN_FACTOR = 3.0f; 
// ==========================================================

volatile uint16_t raw_buffer[NUM_SAMPLES];
volatile int16_t processed_buffer[NUM_SAMPLES];
volatile uint32_t index_gravacao = 0;

repeating_timer_t timer;

void pwm_init_buzzer_synced(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 3.815f);
    pwm_config_set_wrap(&config, 1023); // Resolução de 10 bits
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0);
}

bool amostragem_callback(repeating_timer_t *t) {
    if (index_gravacao >= NUM_SAMPLES) return false;
    raw_buffer[index_gravacao++] = adc_read();
    return true;
}

void play_audio_synced(uint pin, int16_t dc_offset) {
    status_reproducao = true;
    for (uint32_t i = 0; i < NUM_SAMPLES; i++) {
        int32_t pwm_value = processed_buffer[i] + dc_offset;
        uint16_t pwm_value_10bit = pwm_value >> 2;
        if (pwm_value_10bit > 1023) pwm_value_10bit = 1023;
        pwm_set_gpio_level(pin, pwm_value_10bit);
        sleep_us(SAMPLE_INTERVAL_US);
        watchdog_update();
    }
    pwm_set_gpio_level(pin, 0);
    status_reproducao = false;
    printf("Reprodução finalizada.\n");
}

int main() {
    set_sys_clock_khz(125000, true);
    stdio_init_all();
    sleep_ms(2000); 

    watchdog_enable(6000, 1);
    // Inicializacao Microfone
    adc_init();
    adc_gpio_init(MIC_PIN);
    adc_select_input(CANAL_ADC);
    pwm_init_buzzer_synced(BUZZER_PIN);
    // Inicializao de botao
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    printf("Gravando...\n");
    add_repeating_timer_us(-SAMPLE_INTERVAL_US, amostragem_callback, NULL, &timer);

    while (index_gravacao < NUM_SAMPLES) {
        watchdog_update();
        tight_loop_contents();
    }
    cancel_repeating_timer(&timer);
    printf("Gravação encerrada.\n");
    
    // --- DSP ---
    printf("Aplicando filtros e ganho...\n");
    long long sum = 0;
    for (uint32_t i = 0; i < NUM_SAMPLES; i++) sum += raw_buffer[i];
    int16_t dc_offset = sum / NUM_SAMPLES;
    
    const float ALPHA = 0.05f;
    processed_buffer[0] = raw_buffer[0] - dc_offset;
    for (uint32_t i = 1; i < NUM_SAMPLES; i++) {
        processed_buffer[i] = (int16_t)( (ALPHA * (raw_buffer[i] - dc_offset)) + ((1.0f - ALPHA) * processed_buffer[i-1]) );
    }

    const int16_t CLIP_LIMIT = 2047; 

    for (uint32_t i = 0; i < NUM_SAMPLES; i++) {
        // Multiplica a amostra pelo fator de ganho
        int32_t boosted_sample = (int32_t)(processed_buffer[i] * GAIN_FACTOR);

        // Proteção contra "clipping" (distorção)
        if (boosted_sample > CLIP_LIMIT) boosted_sample = CLIP_LIMIT;
        if (boosted_sample < -CLIP_LIMIT) boosted_sample = -CLIP_LIMIT;
        
        processed_buffer[i] = (int16_t)boosted_sample;
    }
    printf("Filtros e ganho aplicados.\n");
    
    while (true) {
        if(gpio_get(BUTTON_A) == 0 && !status_reproducao) {
           play_audio_synced(BUZZER_PIN, dc_offset);
        }
        
        sleep_ms(3000);
    }

    return 0;
}
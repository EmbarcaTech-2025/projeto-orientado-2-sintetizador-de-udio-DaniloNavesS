#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "d_oled/ssd1306.h"
#include "display_modulo.h"
#include "config.h"

static ssd1306_t disp;

void display_init() {
    // Inicializa o I2C
    i2c_init(I2C_PORT, 400 * 1000); // 400kHz
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // Inicializa o display
    disp.external_vcc = false;
    ssd1306_init(&disp, 128, 64, 0x3C, I2C_PORT);
    display_clear();
}

void display_clear() {
    ssd1306_clear(&disp);
    ssd1306_show(&disp);
}

void display_show_message(const char* line1, const char* line2) {
    ssd1306_clear(&disp);
    if (line1) {
        ssd1306_draw_string(&disp, 0, 16, 2, line1);
    }
    if (line2) {
        ssd1306_draw_string(&disp, 0, 40, 2, line2);
    }
    ssd1306_show(&disp);
}

void display_show_message_escala1(const char* line1, const char* line2) {
    ssd1306_clear(&disp);
    if (line1) {
        ssd1306_draw_string(&disp, 10, 20, 1, line1);
    }
    if (line2) {
        ssd1306_draw_string(&disp, 10, 32, 1, line2);
    }
    ssd1306_show(&disp);
}

// Esta é a função principal para desenhar a onda!
void display_draw_waveform(const volatile int16_t* buffer, uint32_t start_index, uint32_t len) {
    ssd1306_clear(&disp);

    // Desenha uma linha central como referência
    int mid_y = 32;
    ssd1306_draw_line(&disp, 0, mid_y, 127, mid_y);

    int16_t max_amp = CLIP_LIMIT; // Usa o limite de clipping do config.h

    for (int x = 0; x < 128; ++x) {
        if (x >= len) break;

        // Mapeia a amplitude do áudio (-2047 a 2047) para a altura da tela (0 a 63)
        // O fator 30.0 é um ajuste para a escala vertical (pode ser ajustado)
        int y = mid_y - (int)(buffer[start_index + x] * (30.0f / max_amp));

        // Garante que o y está dentro dos limites da tela
        if (y < 0) y = 0;
        if (y > 63) y = 63;
        
        // Desenha uma linha vertical do centro até a amplitude da amostra
        ssd1306_draw_line(&disp, x, mid_y, x, y);
    }

    ssd1306_show(&disp); // Envia o buffer para o display
}
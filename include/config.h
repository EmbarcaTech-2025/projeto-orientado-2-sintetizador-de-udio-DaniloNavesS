#ifndef CONFIG_H
#define CONFIG_H

// ==========================================================
// CONFIGURAÇÕES GERAIS DO PROJETO
// ==========================================================

// Pinagem
#define MIC_PIN 28
#define CANAL_ADC 2
#define BUZZER_PIN 21
#define BUTTON_A 5
#define BUTTON_B 6
#define LED_REPRODUCAO_PIN 11
#define LED_GRAVACAO_PIN 13
#define I2C_PORT i2c1
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15

// Parâmetros de Gravação
#define DURATION_SEC 7
#define SAMPLE_RATE 8000
#define NUM_SAMPLES (SAMPLE_RATE * DURATION_SEC)
#define SAMPLE_INTERVAL_US (1000000 / SAMPLE_RATE)

// Parâmetros de Processamento
#define GAIN_FACTOR 2.0f
#define HPF_ALPHA 0.05f  // Coeficiente do filtro passa-alta
#define CLIP_LIMIT 2047  // Limite para evitar distorção (clipping)

#endif // CONFIG_H
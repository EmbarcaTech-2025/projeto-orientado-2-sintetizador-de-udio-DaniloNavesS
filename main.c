#include <stdio.h> // Biblioteca padrão
#include <math.h> // Biblioteca de matemática (função "round" foi utilizada)

#include "pico/stdlib.h" // Biblioteca padrão pico
#include "hardware/gpio.h" // Biblioteca de GPIOs
#include "hardware/adc.h" // Biblioteca do ADC
#include "hardware/pwm.h" // Biblioteca do PWM

#include "hardware/clocks.h" // Biblioteca do Clock

#define MIC_PIN 28
#define CANAL 2

#define BUZZER 21

#define BUZZER_FREQUENCY 100

// Definição de uma função para inicializar o PWM no pino do buzzer
void pwm_init_buzzer(uint pin) {
    // Configurar o pino como saída de PWM
    gpio_set_function(pin, GPIO_FUNC_PWM);

    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o PWM com frequência desejada
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096)); // Divisor de clock
    pwm_init(slice_num, &config, true);

    // Iniciar o PWM no nível baixo
    pwm_set_gpio_level(pin, 0);
}

// Definição de uma função para emitir um beep com duração especificada
void beep(uint pin, uint duration_ms) {
    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o duty cycle para 50% (ativo)
    pwm_set_gpio_level(pin, 2048);

    // Temporização
    sleep_ms(duration_ms);

    // Desativar o sinal PWM (duty cycle 0)
    pwm_set_gpio_level(pin, 0);

    // Pausa entre os beeps
    sleep_ms(100); // Pausa de 100ms
}


float leitor_adc() {
    uint16_t result = adc_read();
    uint8_t value = result >> 4;
    return value;
}

uint8_t buffer[10000];

volatile int contador = 0;

void fila_add(uint8_t value) {
  buffer[contador] = value;
  contador++;
}

uint8_t fila_rem() {
  uint8_t var = buffer[contador-1];
  contador--;
  return var;
}

int main() {
  stdio_init_all();

  pwm_init_buzzer(BUZZER);



  sleep_ms(2000);

  printf("Estou funcionando\n");
  // Inicializacao 
  adc_gpio_init(MIC_PIN);
  adc_init();
  adc_select_input(CANAL);

  // Foi para a funcao -> (BUZZER, GPIO_FUNC_PWM);

  int j = 0;
  while(j < 8000) {

    fila_add(leitor_adc());

    sleep_us(100);

    j++;
  }

  for (int i = 0; i < 10; i++)
  {
    
    printf("valor: %d\n", fila_rem());
  }

  beep(BUZZER, 1000);

  printf("Fim da contagem");
  

  return 0;
}
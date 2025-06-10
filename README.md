
# Projetos de Sistemas Embarcados - EmbarcaTech 2025

Autor: **Danilo Naves do Nascimento**

Curso: Residência Tecnológica em Sistemas Embarcados

Instituição: EmbarcaTech - HBr

Brasília, Junho de 2025

---

# Gravador e Reprodutor de Àudio - Raspberry Pi Pico

Este projeto transforma um Raspberry Pi Pico em um dispositivo de gravação e reprodução de áudio digital. Ele captura áudio de um microfone, aplica processamento de sinal digital (DSP) básico e o reproduz em um buzzer, enquanto fornece feedback visual através de LEDs e de um display OLED.

O foco principal do projeto foi a criação de um software robusto e de fácil manutenção através de uma arquitetura modularizada. 

[Apresentação do Gravador em funcionamento](https://www.youtube.com/shorts/QG4u_qGfzfk)

## Funcionalidades

# Gravador e Reprodutor de Áudio para Raspberry Pi Pico

Este projeto transforma um Raspberry Pi Pico em um dispositivo de gravação e reprodução de áudio digital. Ele captura áudio de um microfone, aplica processamento de sinal digital (DSP) básico e o reproduz em um buzzer, enquanto fornece feedback visual através de LEDs e de um display OLED.

O foco principal do projeto foi a criação de um software robusto e de fácil manutenção através de uma arquitetura modularizada.

## Funcionalidades

* **Gravação de Áudio:** Captura de áudio de 7 segundos a uma taxa de amostragem de 8 kHz.
* **Processamento de Sinal (DSP):**
    * Remoção de offset DC do sinal do microfone.
    * Aplicação de um filtro passa-alta (HPF) para reduzir ruídos de baixa frequência.
    * Aplicação de ganho digital para aumentar o volume.
* **Reprodução de Áudio:** Reprodução do áudio processado via PWM em um buzzer passivo.
* **Feedback Visual:**
    * **LED Vermelho:** Acende durante a gravação.
    * **LED Verde:** Acende durante a reprodução.
    * **Display OLED:** Exibe o status atual e a forma de onda (waveform) do áudio durante a reprodução.

## Hardware Necessário

* Raspberry Pi Pico
* Display OLED 128x64 I2C (baseado no chip SSD1306)
* 1x LED Vermelho
* 1x LED Verde (ou de outra cor)
* 2x Botões de pressão (Push Buttons)
* 1x Buzzer Passivo

## Arquitetura de Software e Modularização

O pilar deste projeto é a **Separação de Responsabilidades**. Em vez de um único arquivo monolítico, o código foi dividido em módulos coesos e com baixo acoplamento.

### Descrição dos Módulos

* **`config.h`**: Centraliza todas as constantes do projeto, como pinagem de hardware, taxa de amostragem, duração da gravação e parâmetros de DSP. Alterar qualquer pino ou parâmetro do sistema é tão simples quanto editar este arquivo.

* **`display_module`**: Responsável por toda a interação com o display OLED. Ele encapsula a biblioteca `ssd1306` e fornece uma interface simples e de alto nível, como `display_init()`, `display_show_message()` e `display_draw_waveform()`. Nenhum outro módulo precisa saber como funciona a comunicação I2C ou como desenhar pixels.

* **`audio_module`**: É o coração do projeto. Este módulo é responsável por:
    * Inicializar o ADC, PWM e os timers de hardware.
    * Gravar o áudio do microfone para um buffer.
    * Processar o buffer de áudio com os filtros e ganho.
    * Reproduzir o áudio processado.
    * Gerenciar o estado interno (gravando, reproduzindo, etc.) e controlar os LEDs de status.
    * Fornecer funções "getter" seguras (`audio_get_buffer_ptr`, `audio_get_playback_index`) para que outros módulos possam consultar seu estado sem modificá-lo.

* **`main.c`**: Atua como o "orquestrador". Sua responsabilidade é mínima e de alto nível:
    * Inicializar o sistema.
    * Gerenciar o fluxo principal do programa (gravar -> processar -> aguardar).
    * No loop principal (`while(true)`), ele apenas verifica o estado dos botões e a lógica de atualização do display, delegando todas as tarefas complexas aos respectivos módulos.

### Arquitetura de Tempo Real com Timers

A qualidade do áudio depende de um timing preciso. Para evitar falhas e jitter, o projeto utiliza **interrupções de hardware (timers)** para as tarefas críticas:

1.  **Gravação (`amostragem_callback`)**: Um `repeating_timer` é configurado para disparar a cada 125µs (8 kHz). Sua única tarefa é ler o ADC e salvar no buffer. Por ser uma operação extremamente rápida e gerenciada por hardware, garantimos que nenhuma amostra seja perdida.

2.  **Reprodução (`playback_callback`)**: Da mesma forma, um segundo `repeating_timer` é usado para a reprodução. A cada 125µs, ele envia uma amostra para o PWM. Isso **desacopla o timing do áudio** de outras tarefas lentas. O loop principal em `main.c` pode estar ocupado atualizando o display OLED, mas o áudio continua a ser reproduzido perfeitamente em segundo plano, sem falhas ou atrasos.

## Possíveis Melhorias

* **Armazenamento em Memória Flash:** Salvar o último áudio gravado na memória flash interna do Pico para que ele persista após o desligamento.
* **Controle de Volume:** Adicionar um potenciômetro ou mais botões para ajustar o `GAIN_FACTOR` em tempo real.
* **Efeitos de Áudio:** Implementar outros filtros de DSP, como eco (delay) ou reverb.
* **Menu no Display:** Criar um sistema de menu no OLED para selecionar diferentes gravações ou efeitos.

---

## 📜 Licença
GNU GPL-3.0.


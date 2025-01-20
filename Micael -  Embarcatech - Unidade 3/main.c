// ---------------------------------------------------------------
//                      TAREFA 3 - Embarcatech
//    Automação de um Sinal de Trânsito para Deficientes Visuais
//                     Aluno: Micael Balza
//                 Linguagem C com Raspberry Pi Pico
// ---------------------------------------------------------------


// --------------------------Explicação----------------------------
//   Criação de 2 estados, um para pedestres e outro para veiculos. 
//   Durante o estado dos veiculos, um led vermelho indica a proi-
//   bição de travessia por pedestres. Uma função, denominada como
//   'is_button_pressed' identifica o acionamento do botão. Caso 
//   o botão seja pressionado, o estado de pedestres é ativado. Ao
//   final do estado de pedestre, o algoritmo volta ao estado de 
//   veículo. 
// ---------------------------------------------------------------


#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

// Define os pinos
#define RED 15       // LED vermelho carros
#define YELLOW 14    // LED amarelo carros
#define GREEN 13     // LED verde carros
#define PED_RED 12   // LED vermelho pedestres
#define PED_GREEN 11 // LED verde pedestres
#define BUZZER 10    // Buzzer (PWM)
#define BUTTON 9     // Botão pedestre

// Define os estados do semáforo
typedef enum {
    ESTADO_VEICULOS,
    ESTADO_PEDESTRES
} Estado;

// Variável global para controlar o estado atual
Estado estado_atual = ESTADO_VEICULOS;

// Função para configurar o PWM no pino do buzzer
void buzzer_pwm_init(uint slice_num) {
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0); // Define o divisor de clock
    pwm_init(slice_num, &config, true);  // Inicializa o PWM
    pwm_set_enabled(slice_num, false);  // Desabilita o PWM inicialmente
}

// Função para gerar um tom no buzzer
void buzzer_tone(uint slice_num, uint freq, uint duration_ms) {
    uint32_t wrap = 125000000 / (4 * freq); // Calcula o topo do contador
    pwm_set_wrap(slice_num, wrap);
    pwm_set_gpio_level(BUZZER, wrap / 2);  // Define duty cycle de 50%
    pwm_set_enabled(slice_num, true);     // Ativa o PWM
    sleep_ms(duration_ms);
    pwm_set_enabled(slice_num, false);    // Desativa o PWM após o som
}

// Função para resetar todos os LEDs
void reset_leds() {
    gpio_put(RED, 0);
    gpio_put(YELLOW, 0);
    gpio_put(GREEN, 0);
    gpio_put(PED_RED, 0);
    gpio_put(PED_GREEN, 0);
}

// Função para gerenciar o estado dos veículos
void estado_veiculos() {
    reset_leds();
    gpio_put(PED_RED, 1); // LED vermelho de pedestres ligado

    // Semáforo verde para veículos
    gpio_put(GREEN, 1);
    for (int i = 0; i < 8000 / 100; i++) {
        sleep_ms(100);
        if (gpio_get(BUTTON) == 0) { // Verifica botão pressionado
            estado_atual = ESTADO_PEDESTRES;
            return; // Sai do estado atual
        }
    }
    gpio_put(GREEN, 0);

    // Semáforo amarelo para veículos
    gpio_put(YELLOW, 1);
    for (int i = 0; i < 2000 / 100; i++) {
        sleep_ms(100);
        if (gpio_get(BUTTON) == 0) {
            estado_atual = ESTADO_PEDESTRES;
            return;
        }
    }
    gpio_put(YELLOW, 0);

    // Semáforo vermelho para veículos
    gpio_put(RED, 1);
    for (int i = 0; i < 10000 / 100; i++) {
        sleep_ms(100);
        if (gpio_get(BUTTON) == 0) {
            estado_atual = ESTADO_PEDESTRES;
            return;
        }
    }
    gpio_put(RED, 0);
}

// Função para gerenciar o estado dos pedestres
void estado_pedestres(uint slice_num) {
    reset_leds();

    // Amarelo para veículos (transição)
    gpio_put(YELLOW, 1);
    sleep_ms(5000);
    gpio_put(YELLOW, 0);

    // Vermelho para veículos e verde para pedestres
    gpio_put(RED, 1);
    gpio_put(PED_RED, 0);     // Desliga vermelho para pedestres
    gpio_put(PED_GREEN, 1);   // Liga verde para pedestres

    // Buzzer ativo durante 15 segundos
    for (int i = 0; i < 15; i++) {
        buzzer_tone(slice_num, 1000, 500); // Tom de 1000 Hz por 500 ms
        sleep_ms(500);
    }

    // Finaliza o estado de pedestres
    reset_leds();
    gpio_put(PED_RED, 1); // Liga vermelho para pedestres
}

// Função para verificar se o botão foi pressionado com debounce
bool is_button_pressed() {
    if (gpio_get(BUTTON) == 0) { // Botão pressionado
        sleep_ms(50); // Debounce
        if (gpio_get(BUTTON) == 0) {
            return true;
        }
    }
    return false;
}

int main() {
    // Inicializa GPIOs
    stdio_init_all();
    gpio_init(RED);
    gpio_init(YELLOW);
    gpio_init(GREEN);
    gpio_init(PED_RED);
    gpio_init(PED_GREEN);
    gpio_init(BUTTON);

    gpio_set_dir(RED, GPIO_OUT);
    gpio_set_dir(YELLOW, GPIO_OUT);
    gpio_set_dir(GREEN, GPIO_OUT);
    gpio_set_dir(PED_RED, GPIO_OUT);
    gpio_set_dir(PED_GREEN, GPIO_OUT);
    gpio_set_dir(BUTTON, GPIO_IN);

    gpio_pull_up(BUTTON); // Configura pull-up para o botão

    // Configura PWM para o buzzer
    gpio_set_function(BUZZER, GPIO_FUNC_PWM); // Define o pino como PWM
    uint slice_num = pwm_gpio_to_slice_num(BUZZER);
    buzzer_pwm_init(slice_num);

    // Estado inicial: LED vermelho para pedestres ligado
    gpio_put(PED_RED, 1);

    while (1) {
        if (estado_atual == ESTADO_VEICULOS) {
            estado_veiculos();
        } else if (estado_atual == ESTADO_PEDESTRES) {
            estado_pedestres(slice_num);
            estado_atual = ESTADO_VEICULOS; // Retorna ao estado inicial
        }
    }
}





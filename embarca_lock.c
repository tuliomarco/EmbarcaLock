// Bibliotecas de uso geral
#include <stdio.h>
#include "pico/stdlib.h"

// Bibliotecas de interação visual com o usuário
#include "ui/matrix_animations.h"
#include "ui/display_manager.h"
#include "ui/user_choose.h"

// Bibliotecas de processamento de informação
#include "core/password_manager.h"
#include "core/sound_manager.h"

// Definição dos pinos para conexão com os LEDs Verde e Vermelho
#define LED_R_PIN 13 // Vermelho (Trancado)
#define LED_G_PIN 11 // Verde (Aberto)

// Variáveis de controle
volatile uint32_t last_interrupt_time = 0; // Controle do efeito bouncing
volatile uint32_t press_start_time = 0; // Controle do modo de redefinição de senha

display_t ssd; // Inicializa a estrutura do display

// Funções definidas no arquivo principal
void init_gpio();
bool check_adm_mode();

int main() {
    stdio_init_all();
    init_gpio();
    matrix_init();
    
    user_choose_init(); // Inicializa as estruturas utilizadas para abas de escolha do usuário
    
    // Declaração dos valores principais: A senha e o seu tamanho (3 || 4 || 5)
    uint8_t* password = malloc(PASSWORD_MAX_SIZE);
    uint8_t size = 0;
    
    // Abertura do prorama
    display_draw_logo(&ssd);
    matrix_begin();

    /* Fluxo de inicialização */
    if(!load_password(password, &size)) { // Verifica se não existe senha 
        sleep_ms(2000); // Aguarda um tempo na tela de abertura

        password = NULL; // Assegura que a senha esteja desreferenciada
        matrix_clear(); // Limpa LEDs da matriz que foram ativadas pela abertura

        while(!password) password = get_password(&ssd); // Repete o processo até uma nova senha ser definida

        size = get_size(); // Busca senha defina
        save_password(password, size); // Salva a senha e o seu tamanho

    } else if(check_adm_mode()) { // Caso tenha uma senha, verifica se o usuário deseja entrar no modo Administrador
        // Sai do estado de abertura
        matrix_clear();
        display_fill(&ssd, false);

        bool new_setted = false;

        while(!new_setted) { // Repete até uma nova senha ser definida
            display_draw_string(&ssd, "ANT", 2, 2); // Inidica que é para digitar a senha ANTerior
            if(user_password_confirmation(&ssd, password, size, false)) { // Verifica se a senha está correta

                uint8_t* new = malloc(PASSWORD_MAX_SIZE); // Aloca espaço para uma nova senha
                new = get_password(&ssd);

                if(!new) { // Se a senha retornar nula, indica que o usuário deseja resetar o programa 
                    clear_password(password); // Limpa senha
                    
                    // Chama novamente a abertura 
                    display_draw_logo(&ssd);
                    matrix_begin();
                    sleep_ms(2000);
                }
                
                matrix_clear();
                while(!new) new = get_password(&ssd); // Repete enquanto uma nova senha não é definida
                size = get_size();

                save_password(new, size);
                password = new;
                new_setted = true;

            } else if (!get_back()) { // Caso a senha não tenha sido confirmada
                // Emite erro
                display_error(&ssd);
                display_fill(&ssd, false);
            }
        }
    }

    matrix_clear();
    /* Fluxo principal */
    while(true) {
        if(get_locked()) { // Fechado 
            gpio_put(LED_R_PIN, true); // Liga led vermelho
            gpio_put(LED_G_PIN, false); // Desliga led verde

            display_fill(&ssd, false);

            if(user_password_confirmation(&ssd, password, size, false)) { // Verifica se a senha foi digitada corretamente
                display_fill(&ssd, false);

                // Emite mensagem e sinal sonoro de sucesso
                display_draw_success(&ssd);
                success_tone();

                set_locked(false); // Entra em estado aberto
                gpio_put(LED_R_PIN, false); // Desliga led vermelho
                gpio_put(LED_G_PIN, true); // liga led verde

                matrix_success(); // Animação de sucesso na matriz
            } else if (!get_back()) { // Caso a senha tenha sido digitada incorretamente
                // Emite erro
                display_error(&ssd);
            } 
            if(!get_back()) sleep_ms(1000);
        }
    }   
}

void init_gpio() { /* Inicializa as GPIOs */ 
    init_buttons();
    init_buzzers();
    init_i2c_display(&ssd);

    gpio_init(LED_R_PIN);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
}

uint32_t time_ms() { /* Função auxiliar para melhorar legibilidade */
    return to_ms_since_boot(get_absolute_time());
}

bool check_adm_mode() { /* Checa pressionamento do botão A na inicialização para entrar no modo Administrador */
    uint32_t press_start = 0;
    uint32_t start_time = time_ms();
    
    while (time_ms() - start_time < 2000) { // Tempo válido para pressionamento (2s)
        if (gpio_get(BTN_A_PIN) == 0) { // Se o botão for pressionado
            if (press_start == 0) {
                press_start = time_ms(); // Marca o início do pressionamento
            }
        } else press_start = 0; // Se soltar, reseta o tempo
    }

    // Se o botão ainda estiver pressionado após o tempo da logo, esperar até completar o tempo necessário
    while (press_start && (time_ms() - press_start < RESET_HOLD_TIME)) {
        if (gpio_get(BTN_A_PIN) != 0) {
            return false; // Se soltar antes do tempo necessário, não entra no modo Adminstrador
        }
    }

    return press_start != 0; // Retorna verdadeiro se manteve pressionado pelo tempo necessário
}
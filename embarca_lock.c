#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/i2c.h"
#include "hardware/adc.h"

#include "ui/display_manager.h"
#include "ui/user_choose.h"
#include "ui/matrix_animations.h"

#include "core/password_manager.h"
#include "core/sound_manager.h"

#define RESET_HOLD_TIME 5000
#define LED_R_PIN 13
#define LED_G_PIN 11

// Controle do efeito bouncing
volatile uint32_t last_interrupt_time = 0;

// Controle do modo de redefinição de senha
volatile uint32_t press_start_time = 0;

void init_gpio() { /* Inicializa as GPIOs */ 
    gpio_init(BTN_A_PIN);
    gpio_init(BTN_B_PIN);
    gpio_init(LED_R_PIN);
    gpio_init(LED_G_PIN);

    gpio_init(BUZZER_PIN);
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    
    gpio_pull_up(BTN_A_PIN);
    gpio_pull_up(BTN_B_PIN);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

display_t ssd; /* Inicializa a estrutura do display */ 
void init_display() { 
    display_init(&ssd, WIDTH, HEIGHT, false, ADDRESS, I2C_PORT); // Inicializa o display
    display_config(&ssd); // Configura o display
    display_send_data(&ssd); // Envia os dados para o display

    // Limpa o display
    display_fill(&ssd, false);
    display_send_data(&ssd);  
}

uint32_t ms_time() {
    return to_ms_since_boot(get_absolute_time());
}

bool check_adm_mode() {
    uint32_t press_start = 0;
    uint32_t start_time = ms_time();
    
    while (ms_time() - start_time < 2000) { // Tempo da logo (2s)
        if (gpio_get(BTN_A_PIN) == 0) { // Se o botão for pressionado
            if (press_start == 0) {
                press_start = ms_time(); // Marca o início do pressionamento
            }
        } else press_start = 0; // Se soltar, reseta o tempo
    }

    // Se o botão ainda estiver pressionado após o tempo da logo, esperar até completar o tempo necessário
    while (press_start && (ms_time() - press_start < RESET_HOLD_TIME)) {
        if (gpio_get(BTN_A_PIN) != 0) {
            return false; // Se soltar antes do tempo necessário, não entra no modo reset
        }
    }

    return press_start != 0; // Retorna verdadeiro se manteve pressionado pelo tempo necessário
}

int main() {
    stdio_init_all();
    i2c_init(I2C_PORT, 400 * 1000);
    init_gpio();
    init_display();
    
    matrix_init(LED_MTX_PIN);
    matrix_clear();
    user_choose_init();

    uint8_t* password = malloc(PASSWORD_SIZE);
    uint8_t size = 0;

    display_draw_logo(&ssd);
    matrix_begin();

    if(!load_password(password, &size)) {
        sleep_ms(2000);
        password = NULL;
        matrix_clear();
        while(!password) password = get_password(&ssd); 

        size = get_size();
        save_password(password, size);

    } else if(check_adm_mode()) {
        matrix_clear();
        display_fill(&ssd, false);

        bool new_setted = false;

        while(!new_setted) {
            display_draw_string(&ssd, "ANT", 2, 2);
            if(user_password_confirmation(&ssd, password, size, false)) {

                uint8_t* new = malloc(PASSWORD_SIZE);
                new = get_password(&ssd);

                if(!new) {
                    clear_password(password);
                    display_draw_logo(&ssd);
                    matrix_begin();
                    sleep_ms(2000);
                }
                
                matrix_clear();
                while(!new) new = get_password(&ssd); 

                size = get_size();
                save_password(new, size);
                password = new;
                new_setted = true;
            } else if (!get_back()) {
                display_error(&ssd);
                display_fill(&ssd, false);
            }
        }
    }

    matrix_clear();
    while(true) {
        if(get_locked()) {
            gpio_put(LED_R_PIN, true);
            gpio_put(LED_G_PIN, false);

            display_fill(&ssd, false);

            if(user_password_confirmation(&ssd, password, size, false)) {
                display_fill(&ssd, false);

                display_draw_success(&ssd);
                success_tone();

                set_locked(false);
                gpio_put(LED_R_PIN, false);
                gpio_put(LED_G_PIN, true);

                matrix_success();
            } else if (!get_back()) {
                display_error(&ssd);
            } 

            if(!get_back()) sleep_ms(1000);
        }
    }   
}
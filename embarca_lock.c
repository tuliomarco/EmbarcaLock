#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ui/display_manager.h"
#include "ui/user_choose.h"

// Controle do efeito bouncing
#define DEBOUNCE_DELAY_MS 200
volatile uint32_t last_interrupt_time = 0;

void init_gpio() { /* Inicializa as GPIOs */ 
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

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

int main() {
    stdio_init_all();
    i2c_init(I2C_PORT, 400 * 1000);
    init_gpio();
    init_display();

    user_choose_init(); // Inicializa as estruturas utilizadas para abas de escolha do usuário
    uint8_t size = user_password_size(&ssd); // Obtendo o tamanho de senha escolhido
    uint8_t* password = user_password(&ssd, size); // Obtendo a senha definida

    // Visualização da senha no terminal para monitoramento 
    for(int i = 0; i < size; i++) printf("%u ", password[i]);
}
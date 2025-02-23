#include <stdio.h>
#include "pico/stdlib.h"
#include "display_manager.h"
#include "hardware/adc.h"
#include "user_choose.h"

extern uint32_t last_interrupt_time; // Tratamento do bouncing

// Variáveis modificadas nas interrupções
volatile bool size_selected = false;
volatile bool number_selected = false;
volatile bool confirmed = false;
volatile bool decrease = false;
volatile int numbers_entered = 0;

// Valores para formação da senha
uint8_t size = 0;
uint8_t button_selected = 0;
uint8_t password[MAX_PASSWORD_SIZE] = {}; 


void user_choose_init() { /* Inicialização das GPIOs utilizadas e configuração das interrupções*/
    gpio_init(BTN_JSTCK_PIN);
    gpio_init(BTN_A_PIN);
    gpio_init(BTN_B_PIN);

    gpio_set_dir(BTN_JSTCK_PIN, GPIO_IN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);

    gpio_pull_up(BTN_JSTCK_PIN);
    gpio_pull_up(BTN_A_PIN);
    gpio_pull_up(BTN_B_PIN);

    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);

    gpio_set_irq_enabled_with_callback(BTN_JSTCK_PIN, GPIO_IRQ_EDGE_FALL, true, &select_callback);
    gpio_set_irq_enabled_with_callback(BTN_A_PIN, GPIO_IRQ_EDGE_FALL, true, &select_callback);
    gpio_set_irq_enabled_with_callback(BTN_B_PIN, GPIO_IRQ_EDGE_FALL, true, &select_callback);
}

/* Funções de controle para seleção da senha e de seu tamanho */
bool size_selected_condition(void) {
    return size_selected;
}
bool number_selected_condition(void) {
    return number_selected;
}

void select_callback(uint gpio, uint32_t events) { /* Callcack para pressionamento dos botões */
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if(now - last_interrupt_time > DEBOUNCE_DELAY_MS) {
        last_interrupt_time = now;

        if(size_selected) {
            if(gpio == 22 && numbers_entered < size) { // Seleção dos botões digitais pelo Joystick
                numbers_entered++;
            } else if (gpio == 5 && numbers_entered > 0) { // Desfazer
                numbers_entered--;
                decrease = true;
            } else if (gpio == 6 && numbers_entered == size) { // Confirmação
                confirmed = true;
            }
            number_selected = true;
        } else if(gpio == 22) size_selected = true;
    }
}

uint8_t user_password_size(display_t *ssd) { /* Aba de escolha do tamanho da senha */
    uint8_t x = 36, y = 26;
    for(int i = 0; i < 3; i++) {
        display_draw_number_button(ssd, i + 3, x + (20 * i), y, !i);
    }
    display_send_data(ssd);

    uint8_t options[] = {3, 4, 5};
    uint8_t size = user_select_option(ssd, 3, options, x, y, &size_selected_condition);

    display_fill(ssd, false);
    button_selected = 0;
    return size;
}

void user_password_monitoring(display_t *ssd, uint8_t size) { /* Visor de preenchimento da senha */
    int x = 37 + (5 - size) * 6; // Ajusta o x inicial baseado no size
    int y = 4;

    for(int i = 0; i < size; i++) {
        display_rect(ssd, y, x, 9, 9, true, i+1 <= numbers_entered);
        x += 12;
    }
}

uint8_t* user_password(display_t *ssd, uint8_t password_size) { /* Aba para seleção da senha */
    size = password_size;
    user_password_monitoring(ssd, size);

    // Desenho inicial do teclado digital
    int x = 16, y = 23;
    for(int i = 0; i < 2; i++) {
        for(int j = 0; j < 5; j++) {
            display_draw_number_button(ssd, j + (5 * i), x, y, (j + (5 * i)) == button_selected);
            x += SPACE_BETWEEN_X;
        }
        x = 16;
        y += SPACE_BETWEEN_Y;
    }
    display_send_data(ssd);
    uint8_t options[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    // Seleção dos valores da senha
    uint8_t number;
    bool full = false;
    while(!confirmed) {
        number = user_select_option(ssd, 10, options, 16, 23, &number_selected_condition);

        if(numbers_entered <= size && numbers_entered > 0 && !confirmed) {
            password[numbers_entered - 1] = decrease || full ? password[numbers_entered - 1] : number;
            full = (numbers_entered == size);
        } 
        decrease = false;
        number_selected = false;

        user_password_monitoring(ssd, size);
        display_send_data(ssd);

        sleep_ms(150);
    }
    return password;
}

uint8_t user_select_option(display_t *ssd, uint8_t options_amount, uint8_t* options,
    uint8_t init_x, uint8_t init_y, bool (*stop_condition)()) { /* Navegação e seleção de números no teclado digital */ 
    
    uint16_t vrx_value, vry_value;

    while (!stop_condition()) {
        adc_select_input(0);
        vry_value = adc_read();
        adc_select_input(1);
        vrx_value = adc_read();

        button_selected = user_change_select_button(ssd, vrx_value, vry_value, init_x, init_y, 
            button_selected, options_amount, options);
        sleep_ms(100);
    }
    return options[button_selected];
}

uint8_t user_change_select_button(display_t *ssd, uint16_t vrx, uint16_t vry,
    uint8_t init_x, uint8_t init_y, uint8_t selected, uint8_t max, uint8_t* options) { /* Desenho do teclado digital */ 

    uint16_t x = init_x + (SPACE_BETWEEN_X * (selected % 5)); 
    uint16_t y = init_y + (SPACE_BETWEEN_Y * (selected / 5));

    display_draw_number_button(ssd, options[selected], x, y, false);

    // Movimentação do joystick
    if(vrx < 1000) x -= selected % 5 > 0 ? SPACE_BETWEEN_X : -(SPACE_BETWEEN_X * ((max - 1) % 5));  // Ajustar movimento no eixo X
    else if(vrx > 3000) x += selected % 5 < ((max - 1) % 5) ? SPACE_BETWEEN_X : -(SPACE_BETWEEN_X * ((max - 1) % 5));

    if(vry < 1000 && max >= 5) y = init_y + SPACE_BETWEEN_Y;  // Ajustar movimento no eixo Y
    else if(vry > 3000 && max >= 5) y = init_y;

    selected = ((x - init_x) / SPACE_BETWEEN_X) + 5 * ((y - init_y) / SPACE_BETWEEN_Y);  // Recalcular a posição do botão selecionado

    // Desenhar o novo botão na posição correta
    display_draw_number_button(ssd, options[selected], x, y, true);
    display_send_data(ssd);

    return selected;
}

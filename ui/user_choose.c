#include "pico/stdlib.h"
#include "display_manager.h"
#include "hardware/adc.h"
#include "user_choose.h"
#include "password_manager.h"
#include "matrix_animations.h"
#include "core/sound_manager.h"

extern uint32_t last_interrupt_time; // Tratamento do bouncing

// Variáveis modificadas nas interrupções
volatile bool size_selected = false;
volatile bool number_selected = false;
volatile bool confirmed = false;
volatile bool decrease = false;
volatile bool back = false;
volatile bool locked = true;
volatile int numbers_entered = 0;
volatile int reset_counter = 0;

// Valores para formação da senha
uint8_t size = 0;
uint8_t button_selected = 0;

void init_buttons() {
    gpio_init(BTN_A_PIN);
    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);
    gpio_pull_up(BTN_B_PIN);
}

void user_choose_init() { /* Inicialização das GPIOs utilizadas e configuração das interrupções*/
    gpio_init(BTN_JSTCK_PIN);
    gpio_set_dir(BTN_JSTCK_PIN, GPIO_IN);
    gpio_pull_up(BTN_JSTCK_PIN);

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

/* Getters e Setters */
uint8_t get_size(void) {
    return size;
}
uint8_t get_back(void) {
    return back;
}
uint8_t get_locked(void) {
    return locked;
}
uint8_t set_locked(bool value) {
    locked = value;
}

void reset_values() { /* Reiniciar valores para novas instâncias */
    reset_counter = 0;
    number_selected = false;
    confirmed = false;
    decrease = false;
    numbers_entered = 0;
    size = 0;
    button_selected = back && size_selected ? button_selected : 0; // Só permanece no mesmo botão se for uma transição CFM -> DEF    
    back = false;
}

void select_callback(uint gpio, uint32_t events) { /* Callcack para pressionamento dos botões */
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if(now - last_interrupt_time > DEBOUNCE_DELAY_MS) {
        last_interrupt_time = now;

        if(size_selected) { // Verifica se já passou pela escolha do tamanho
            if(gpio == 22 && numbers_entered < size) { // Seleção dos botões digitais pelo Joystick
                numbers_entered++;
            } else if (gpio == 5 && numbers_entered >= 0) { // Desfazer
                if(numbers_entered == 0) {
                    back = true; // Se não tiver número algum, o usuário deseja voltar para a página anterior
                }
                else numbers_entered--;
                decrease = true;

            } else if (gpio == 6) { // Confirmação
                if(numbers_entered == size && locked) confirmed = true;
                else if(!locked) locked = true; // Ao confirmar a senha, passa para o estado trancado
            }
            number_selected = true;

        } else if(gpio == 22) { // Caso não tenha tamanho definido
            size_selected = true; // O pressionamento do botão do Joystick seleciona o tamanho
        } else if(gpio == 5) {
            reset_counter++;
            if(reset_counter == RESET_PATTERN_CLICKS) { // Caso o botão A seja pressionado RESET_PATTERN_CLICKS vezes na seleção do tamanho
                size_selected = true; // O size_selected passa a ser true para o tamanho e a senha retornarem nulos e entrar em modo reset
            }
        }
    }
}

uint8_t user_password_size(display_t *ssd) { /* Aba de escolha do tamanho da senha */
    reset_values();
    size_selected = 0;
    display_fill(ssd, false);

    uint8_t x = 36, y = 24;
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
    int y = 10;

    for(int i = 0; i < size; i++) {
        display_rect(ssd, y, x, 9, 9, true, i+1 <= numbers_entered);
        x += 12;
    }
}

uint8_t* user_password(display_t *ssd, uint8_t password_size) { /* Aba para seleção da senha */
    uint8_t* password = malloc(password_size * sizeof(uint8_t));

    reset_values();
    size_selected = true;
    size = password_size;
    user_password_monitoring(ssd, size);

    // Desenho inicial do teclado digital
    int x = 16, y = 27;
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
    while(!confirmed && !back) {
        number = user_select_option(ssd, 10, options, 16, 27, &number_selected_condition);

        if(decrease) {
            if(numbers_entered > 0) matrix_number(password[numbers_entered - 1], 1, 0, 0);
            else matrix_clear();
        }

        if(numbers_entered <= size && numbers_entered > 0 && !confirmed) {
            password[numbers_entered - 1] = decrease || full ? password[numbers_entered - 1] : number;
            
            if (!full) matrix_number(number, 1, 0, 0);

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

bool user_password_confirmation(display_t *ssd, uint8_t *password, const uint8_t size, bool defining) { /* Função para confirmação da senha */
    uint8_t *confirmation_password, j;
    bool equal = false;

    for(int i = 0; i < MAX_ATTEMPTS; i++) { // Confere a senha digitada por MAX_ATTEMPTS vezes
        if(!defining) display_draw_char(ssd, '3' - i, 116, 2); // Em modo de definição não é preciso contagem para bloqueio
        confirmation_password = user_password(ssd, size);

        if(back && defining) break; // Se em modo de definição o usuário quiser retornar, ele pode;
        else if (back) { // Se uma senha já foi definida, ele não pode retornar, precisa ser mantido na confirmação
            i--; // Permance com a mesma quantidade de tentativas sobrando
            continue;
        }

        for(j = 0; j < size; j++) if(confirmation_password[j] != password[j]) { // Verifica se a senha está incorreta
            if(!defining) error_tone();
            break;
        }

        free(confirmation_password);

        matrix_clear();

        if(j == size) { // Verifica se a senha está correta
            equal = true;
            break;
        }
    }
    return equal;
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

uint8_t* get_password(display_t *ssd) { /* Função que retorna a senha definida pelo usuário */
    uint8_t screen = 0; // Variável que determina em qual tela o usuário está
    bool setted = false; // Representa a definição ou não definição da senha

    uint8_t size;
    uint8_t* password;

    while(!setted) {
        switch (screen) { // Navega entre as telas de seleção
        case 0: // Tela 1: Escolha do tamanho da senha
            size = user_password_size(ssd);
            // Verifica se o botão A foi pressionado RESET_PATTERN_CLICKS na tela de tamanho (Reset Mode)
            if(reset_counter == RESET_PATTERN_CLICKS) { 
                password = NULL; // Retorna senha nula para indicar a intenção de reset
                size_selected = false;
                reset_counter = 0;
                setted = true;
            } else screen++;
            break;
        case 1: // Tela 2: Definição da senha
            display_fill(ssd, false);
            display_draw_string(ssd, "DEF", 2, 2); // Indica definição de senha
            password = user_password(ssd, size);
            if(back) { // Verifica se deseja retornar para a tela anterior
                size_selected = false;
                screen--;
            }
            else screen++; // Passa para a tela seguinte
            matrix_clear();
            break;
        case 2: // Tela 3: Confirmação da senha
            display_fill(ssd, false);
            display_draw_string(ssd, "CFM", 2, 2); // Indica confirmação de senha
            setted = user_password_confirmation(ssd, password, size, true);
            if(!setted) { // Verifica se a senha está incorreta
                if(!back) { // Caso o usuário esteja apenas querendo voltar
                    display_fill(ssd, false);
                    display_draw_error(ssd);
                    display_send_data(ssd);
                    matrix_error();
                }
                screen--;
            } else { // Caso a senha esteja correta
                display_fill(ssd, false);
                display_draw_success(ssd);
                display_send_data(ssd);
                sleep_ms(1000);
            }
            break;
        }
    };
    return password;
}

void display_error(display_t *ssd) { /* Função de bloqueio por 30 segundos*/
    display_draw_error(ssd);
    matrix_error();
    display_countdown(ssd, 30);
}

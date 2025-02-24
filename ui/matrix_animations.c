#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "matrix_animations.h"
#include "patterns.h"

#include "ws2812b.pio.h"

// Variáveis para uso da PIO
PIO pio;
uint sm;

// Pix GRB
typedef struct pixel_t {
    uint8_t G, R, B;
} led_t;

led_t led_matrix[LED_MTX_COUNT]; // Buffer de pixels que compôem a matriz

void matrix_init(uint pin) { /* Inicialização da PIO */
    uint offset = pio_add_program(pio0, &ws2812b_program);
    pio = pio0;

    sm = pio_claim_unused_sm(pio, false);

    ws2812b_program_init(pio, sm, offset, pin);
}

uint32_t rgb_value(uint8_t B, uint8_t R, uint8_t G){ /* Formatação do valor GRB */ 
    return (G << 24) | (R << 16) | (B << 8);
  }

void set_led(const uint id, const uint8_t R, const uint8_t G, const uint8_t B) { /* Atribuição de cor a um LED */
    led_matrix[id].R = R;
    led_matrix[id].G = G;
    led_matrix[id].B = B;
}

void clear_leds() { /* Limpeza do buffer de LEDs */
    for (uint i = 0; i < LED_MTX_COUNT; i++) {
        led_matrix[i].R = 0;
        led_matrix[i].G = 0;
        led_matrix[i].B = 0;
    }
}

void write_leds() { /* Transferência dos valores do buffer para a matriz de LEDs */
    uint32_t value;
    for (uint i = 0; i < LED_MTX_COUNT; ++i) {
        value = rgb_value(led_matrix[i].B, led_matrix[i].R, led_matrix[i].G);
        pio_sm_put_blocking(pio, sm, value);
    }
}

void set_led_by_pattern(uint32_t pattern, bool red, bool green, bool blue, bool turn_off_previous) { /* Decodificação do padrão binário para LEDs da matriz */
    uint lvl = LED_MTX_LEVEL;
    for(uint i = 0; i < LED_MTX_COUNT; i++) {
        // Verfica se o bit é 1. Em casos positivos, acende o LED na cor branca com a intensidade setada
        if((pattern >> i) & 1) set_led(i, lvl * red, lvl * green, lvl * blue); 
        else if(turn_off_previous) set_led(i, 0, 0, 0); // Caso contrário, deixa o LED apagado 
    }
}

void matrix_clear() {
    clear_leds();
    write_leds();
}
void matrix_error() {
    for(int i = 0; i < 3; i++) {
        set_led_by_pattern(status[0], 1, 0, 0, true);
        write_leds();
        sleep_ms(600);
        matrix_clear();
        sleep_ms(600);
    }
}

void matrix_success() {
    for(int i = 0; i < 3; i++) { 
        for(int j = 0; j < 3; j++) {
            set_led_by_pattern(status[j+1], 0, 1, 0, true);
            write_leds();
            sleep_ms(150);
        }
        matrix_clear();
        sleep_ms(200);
    }
}

void matrix_number(uint8_t number, bool red, bool green, bool blue) {
    set_led_by_pattern(led_number_pattern[number], red, green, blue, true);
    write_leds();
}
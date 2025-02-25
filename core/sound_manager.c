#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "sound_manager.h"

void init_buzzers() {
    gpio_init(BUZZER_PIN);
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
}

void beep(uint16_t frequency, uint16_t duration) { /* Função para emissão do sinal sonoro */
    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_clkdiv(slice, 2.0f); // Ajuste do clock para maior precisão

    uint32_t clock_freq = clock_get_hz(clk_sys); // Obtém a frequência do clock do sistema
    uint32_t wrap = clock_freq / (frequency * 2);
    pwm_set_wrap(slice, wrap);
    pwm_set_gpio_level(BUZZER_PIN, wrap / 4); // 25% duty cycle para melhor audibilidade
    pwm_set_enabled(slice, true);

    sleep_ms(duration);
    pwm_set_enabled(slice, false);
}

void success_tone() {
    beep(2500, 150); // Tom médio-agudo
}

void error_tone() {
    for (int i = 0; i < 3; i++) {
        beep(1000, 150);  // Tom mais grave
        sleep_ms(50);    // Pequena pausa
    }
}
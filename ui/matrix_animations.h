
#ifndef MATRIX_ANIMATION_H
#define MATRIX_ANIMATION_H

// Definições para uso dos LEDs na Matriz 5x5
#define LED_MTX_COUNT 25
#define LED_MTX_LEVEL 20 // A intesidade está baixa para não causar incômodo (0-255, caso deseje alterar)
#define LED_MTX_PIN 7

void matrix_init(uint pin);
void matrix_number(uint8_t number, bool red, bool green, bool blue);
void matrix_error();
void matrix_success();
void matrix_clear();

#endif
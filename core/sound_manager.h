#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#define BUZZER_PIN 21

void init_buzzers();
void beep(uint16_t frequency, uint16_t duration);
void success_tone();
void error_tone();

#endif
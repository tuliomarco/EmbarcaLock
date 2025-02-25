#ifndef USER_CHOOSE_H
#define USER_CHOOSE_H

#define SPACE_BETWEEN_X 20
#define SPACE_BETWEEN_Y 20

#define BTN_JSTCK_PIN 22
#define BTN_A_PIN 5
#define BTN_B_PIN 6

#define VRX_PIN 27
#define VRY_PIN 26

#define MAX_ATTEMPTS 3

#define DEBOUNCE_DELAY_MS 200
#define RESET_PATTERN_CLICKS 3 

void user_choose_init();
void select_callback(uint gpio, uint32_t events);
uint8_t user_change_select_button(display_t *ssd, uint16_t vrx, uint16_t vry, uint8_t init_x, uint8_t init_y, uint8_t selected, uint8_t max, uint8_t *options);
uint8_t user_select_option(display_t *ssd, uint8_t options_amount, uint8_t *options, uint8_t init_x, uint8_t init_y, bool (*stop_condition)());
uint8_t user_password_size(display_t *ssd);
uint8_t* user_password(display_t *ssd, uint8_t password_size);
void user_password_monitoring(display_t *ssd, uint8_t size);
bool user_password_confirmation(display_t *ssd, uint8_t *password, const uint8_t size, bool defining);
uint8_t* get_password(display_t *ssd);
uint8_t get_size(void);
uint8_t get_back(void);
uint8_t get_locked(void);
uint8_t set_locked(bool value);
void display_error(display_t *ssd);

#endif
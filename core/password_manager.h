#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#define PASSWORD_SIZE 5

typedef struct {
    uint8_t password[PASSWORD_SIZE];
    uint8_t size;
} password_data_t;

void save_password(uint8_t *password, uint8_t size);
bool load_password(uint8_t *password, uint8_t *size);
void clear_password(uint8_t *password);
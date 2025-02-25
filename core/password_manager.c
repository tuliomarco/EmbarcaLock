#include <string.h>
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "password_manager.h"

void save_password(uint8_t *password, uint8_t password_size) {
    password_data_t data;

    // Copiar a senha para a estrutura
    memcpy(data.password, password, password_size);
    data.size = password_size; // Armazenar o tamanho da senha

    uint8_t buffer[FLASH_PAGE_SIZE];
    memset(buffer, 0xFF, FLASH_PAGE_SIZE);  // Inicializa com 0xFF (estado apagado da flash)

    // Copia a estrutura para o buffer
    memcpy(buffer, &data, sizeof(password_data_t));

    // Desativa interrupções para evitar problemas na escrita da flash
    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);  // Apaga o setor antes de escrever
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_PAGE_SIZE);
    restore_interrupts(interrupts);
}


bool load_password(uint8_t *password, uint8_t *password_size) {
    const uint8_t *flash_mem = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);

    // Se a flash estiver apagada (0xFF), significa que não há senha salva
    if (flash_mem[0] == 0xFF) return false;

    // Copia a estrutura da flash para a variável local
    password_data_t data;
    memcpy(&data, flash_mem, sizeof(password_data_t));

    // Copia a senha e o tamanho para as variáveis fornecidas
    memcpy(password, data.password, data.size);  // Copia a senha

    *password_size = data.size;  // Recupera o tamanho da senha

    return true;
}

void clear_password(uint8_t *password) {
    // Limpar a variável de senha na memória volátil
    memset(password, 0xFF, PASSWORD_SIZE); // Pode usar 0x00 também, mas 0xFF é comum para "sem valor"

    // Apagar a senha e o tamanho da memória flash
    uint8_t buffer[FLASH_PAGE_SIZE];  // Um bloco da flash para escrever
    memset(buffer, 0xFF, FLASH_PAGE_SIZE); // Inicializa com 0xFF (estado apagado da flash)

    // Desativa interrupções para evitar problemas na escrita da flash
    uint32_t interrupts = save_and_disable_interrupts();

    // Apaga o setor onde a senha e o tamanho foram salvos
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);  

    // Sobrescreve o setor com 0xFF
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_PAGE_SIZE); // Sobrescreve com 0xFF

    // Restaura as interrupções
    restore_interrupts(interrupts);
}

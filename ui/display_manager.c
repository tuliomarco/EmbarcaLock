#include "display_manager.h"
#include "bitmap.h"
#include "string.h"
#include "font.h"

void display_init(display_t *ssd, uint8_t width, uint8_t height, bool external_vcc, uint8_t address, i2c_inst_t *i2c) {
  ssd->width = width;
  ssd->height = height;
  ssd->pages = height / 8U;
  ssd->address = address;
  ssd->i2c_port = i2c;
  ssd->bufsize = ssd->pages * ssd->width + 1;
  ssd->ram_buffer = calloc(ssd->bufsize, sizeof(uint8_t));
  ssd->ram_buffer[0] = 0x40;
  ssd->port_buffer[0] = 0x80;
}

void display_config(display_t *ssd) {
  display_command(ssd, SET_DISP | 0x00);
  display_command(ssd, SET_MEM_ADDR);
  display_command(ssd, 0x01);
  display_command(ssd, SET_DISP_START_LINE | 0x00);
  display_command(ssd, SET_SEG_REMAP | 0x01);
  display_command(ssd, SET_MUX_RATIO);
  display_command(ssd, HEIGHT - 1);
  display_command(ssd, SET_COM_OUT_DIR | 0x08);
  display_command(ssd, SET_DISP_OFFSET);
  display_command(ssd, 0x00);
  display_command(ssd, SET_COM_PIN_CFG);
  display_command(ssd, 0x12);
  display_command(ssd, SET_DISP_CLK_DIV);
  display_command(ssd, 0x80);
  display_command(ssd, SET_PRECHARGE);
  display_command(ssd, 0xF1);
  display_command(ssd, SET_VCOM_DESEL);
  display_command(ssd, 0x30);
  display_command(ssd, SET_CONTRAST);
  display_command(ssd, 0xFF);
  display_command(ssd, SET_ENTIRE_ON);
  display_command(ssd, SET_NORM_INV);
  display_command(ssd, SET_CHARGE_PUMP);
  display_command(ssd, 0x14);
  display_command(ssd, SET_DISP | 0x01);
}

void display_command(display_t *ssd, uint8_t command) {
  ssd->port_buffer[1] = command;
  i2c_write_blocking(
    ssd->i2c_port,
    ssd->address,
    ssd->port_buffer,
    2,
    false
  );
}

void display_send_data(display_t *ssd) {
  display_command(ssd, SET_COL_ADDR);
  display_command(ssd, 0);
  display_command(ssd, ssd->width - 1);
  display_command(ssd, SET_PAGE_ADDR);
  display_command(ssd, 0);
  display_command(ssd, ssd->pages - 1);
  i2c_write_blocking(
    ssd->i2c_port,
    ssd->address,
    ssd->ram_buffer,
    ssd->bufsize,
    false
  );
}

void display_pixel(display_t *ssd, uint8_t x, uint8_t y, bool value) {
  uint16_t index = (y >> 3) + (x << 3) + 1;
  uint8_t pixel = (y & 0b111);
  if (value)
    ssd->ram_buffer[index] |= (1 << pixel);
  else
    ssd->ram_buffer[index] &= ~(1 << pixel);
}

void display_fill(display_t *ssd, bool value) {
    // Itera por todas as posições do display
    for (uint8_t y = 0; y < ssd->height; ++y) {
        for (uint8_t x = 0; x < ssd->width; ++x) {
            display_pixel(ssd, x, y, value);
        }
    }
}



void display_rect(display_t *ssd, uint8_t top, uint8_t left, uint8_t width, uint8_t height, bool value, bool fill) {
  for (uint8_t x = left; x < left + width; ++x) {
    display_pixel(ssd, x, top, value);
    display_pixel(ssd, x, top + height - 1, value);
  }
  for (uint8_t y = top; y < top + height; ++y) {
    display_pixel(ssd, left, y, value);
    display_pixel(ssd, left + width - 1, y, value);
  }
  for (uint8_t x = left + 1; x < left + width - 1; ++x) {
    for (uint8_t y = top + 1; y < top + height - 1; ++y) {
      display_pixel(ssd, x, y, fill);
    }
  }
}

void display_line(display_t *ssd, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool value) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;

    while (true) {
        display_pixel(ssd, x0, y0, value); // Desenha o pixel atual

        if (x0 == x1 && y0 == y1) break; // Termina quando alcança o ponto final

        int e2 = err * 2;

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}


void display_hline(display_t *ssd, uint8_t x0, uint8_t x1, uint8_t y, bool value) {
  for (uint8_t x = x0; x <= x1; ++x)
    display_pixel(ssd, x, y, value);
}

void display_vline(display_t *ssd, uint8_t x, uint8_t y0, uint8_t y1, bool value) {
  for (uint8_t y = y0; y <= y1; ++y)
    display_pixel(ssd, x, y, value);
}

// Função para desenhar um caractere
void display_draw_char(display_t *ssd, char c, uint8_t x, uint8_t y) {
    uint16_t index = 0;
    if (c >= 'A' && c <= 'Z') {
      index = (c - 'A' + 11) * 8; // Para letras maiúsculas
    } else if (c >= '0' && c <= '9') {
      index = (c - '0' + 1) * 8; // Para números
    } else if (c >= 'a' && c <= 'z') {
      index = (c - 'a' + 37) * 8; // Para letras minúsculas
    }
  
    for (uint8_t i = 0; i < 8; ++i) {
      uint8_t line = font[index + i];
      for (uint8_t j = 0; j < 8; ++j) {
        bool pixel_on = line & (1 << j);
        display_pixel(ssd, x + i, y + j, pixel_on);
      }
    }
  }
  

// Função para desenhar uma string
void display_draw_string(display_t *ssd, const char *str, uint8_t x, uint8_t y)
{
  while (*str)
  {
    display_draw_char(ssd, *str++, x, y);
    x += 8;
    if (x + 8 >= ssd->width)
    {
      x = 0;
      y += 8;
    }
    if (y + 8 >= ssd->height)
    {
      break;
    }
  }
}

void display_draw_bitmap(display_t *ssd, const uint8_t *bitmap, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool fill) {
  uint16_t byte_index = 0;
  for (uint8_t i = 0; i < height; ++i) {
      for (uint8_t j = 0; j < width; ++j) {
          // Encontra o byte correto dentro do bitmap
          uint8_t byte = bitmap[byte_index];
          // Calcula o bit dentro do byte
          bool pixel_on = byte & (1 << (7 - (j % 8))); // Considera o bit correto (da esquerda para a direita)
          
          // Desenha o pixel
          if(pixel_on) display_pixel(ssd, x + j, y + i, fill ? pixel_on : !pixel_on);
          
          // Se já percorremos 8 bits, passe para o próximo byte
          if ((j + 1) % 8 == 0) {
              ++byte_index;
          }
      }
  }
}

void display_draw_number_button(display_t *ssd, uint8_t number, uint8_t x, uint8_t y, bool hover) { /* Função para desenhar botões numéricos */
  display_rect(ssd, y, x, 17, 17, true, !hover);
  display_draw_bitmap(ssd, &numbers[number * 32], x, y, 16, 16, hover);
}

/* Funções para desenhar os status na tela */
void display_draw_success(display_t *ssd) {
  display_draw_bitmap(ssd, success, 32, 0, 64, 64, true);
}
void display_draw_error(display_t *ssd) {
  display_draw_bitmap(ssd, error, 24, 0, 80, 64, true);
}
void display_draw_logo(display_t *ssd) {
  display_draw_bitmap(ssd, logo_embarca_lock, 0, 0, 128, 64, true);
}
#include "gfx_gc9107.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define GC9107_CASET   0x2A
#define GC9107_RASET   0x2B
#define GC9107_RAMWR   0x2C
#define GC9107_MADCTL  0x36
#define GC9107_INVON   0x21
#define GC9107_INVOFF  0x20
#define GC9107_SLPOUT  0x11
#define GC9107_SLPIN   0x10
#define GC9107_SWRESET 0x01
#define GC9107_DISPON  0x29
#define GC9107_COLMOD  0x3A

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_BGR 0x08

static void hw_reset(gfx_gc9107_t *lcd) {
  if (lcd->rst_pin != GPIO_NUM_NC) {
    gpio_set_direction(lcd->rst_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(lcd->rst_pin, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(lcd->rst_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(lcd->rst_pin, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

esp_err_t gc9107_init(gfx_gc9107_t *lcd) {
  hw_reset(lcd);

  gfx_spi_send_cmd(&lcd->spi, GC9107_SWRESET);
  vTaskDelay(pdMS_TO_TICKS(150));

  gfx_spi_send_cmd(&lcd->spi, GC9107_SLPOUT);
  vTaskDelay(pdMS_TO_TICKS(120));

  // 16-bit color
  uint8_t color_mode = 0x05;
  gfx_spi_send_cmd(&lcd->spi, GC9107_COLMOD);
  gfx_spi_send_data(&lcd->spi, &color_mode, 1);

  // Initialize MADCTL default
  gc9107_set_rotation(lcd, 0);

  gfx_spi_send_cmd(&lcd->spi, GC9107_DISPON);
  vTaskDelay(pdMS_TO_TICKS(100));

  lcd->width = 128;
  lcd->height = 128;
  lcd->x_offset = 0;
  lcd->y_offset = 0;

  return ESP_OK;
}

void gc9107_set_rotation(gfx_gc9107_t *lcd, uint8_t rotation) {
  uint8_t madctl;
  lcd->rotation = rotation % 4;

  switch (lcd->rotation) {
    case 1:  madctl = MADCTL_MY | MADCTL_MV | MADCTL_BGR; break;
    case 2:  madctl = MADCTL_BGR; break;
    case 3:  madctl = MADCTL_MX | MADCTL_MV | MADCTL_BGR; break;
    default: madctl = MADCTL_MY | MADCTL_MX | MADCTL_BGR; break;
  }

  gfx_spi_send_cmd(&lcd->spi, GC9107_MADCTL);
  gfx_spi_send_data(&lcd->spi, &madctl, 1);
}

void gc9107_invert_display(gfx_gc9107_t *lcd, bool invert) {
  gfx_spi_send_cmd(&lcd->spi, (lcd->ips ? (invert ? GC9107_INVOFF : GC9107_INVON) : (invert ? GC9107_INVON : GC9107_INVOFF)));
}

void gc9107_display_on(gfx_gc9107_t *lcd) {
  gfx_spi_send_cmd(&lcd->spi, GC9107_SLPOUT);
  vTaskDelay(pdMS_TO_TICKS(100));
}

void gc9107_display_off(gfx_gc9107_t *lcd) {
  gfx_spi_send_cmd(&lcd->spi, GC9107_SLPIN);
  vTaskDelay(pdMS_TO_TICKS(100));
}

static void set_addr_window(gfx_gc9107_t *lcd, int x, int y, int w, int h) {
  uint8_t data[4];

  gfx_spi_send_cmd(&lcd->spi, GC9107_CASET);
  data[0] = 0x00;
  data[1] = x + lcd->x_offset;
  data[2] = 0x00;
  data[3] = (x + w - 1) + lcd->x_offset;
  gfx_spi_send_data(&lcd->spi, data, 4);

  gfx_spi_send_cmd(&lcd->spi, GC9107_RASET);
  data[0] = 0x00;
  data[1] = y + lcd->y_offset;
  data[2] = 0x00;
  data[3] = (y + h - 1) + lcd->y_offset;
  gfx_spi_send_data(&lcd->spi, data, 4);

  gfx_spi_send_cmd(&lcd->spi, GC9107_RAMWR);
}

void gc9107_draw_pixel(gfx_gc9107_t *lcd, int x, int y, uint16_t color) {
  if (x < 0 || y < 0 || x >= lcd->width || y >= lcd->height) {
    return;
  }
  set_addr_window(lcd, x, y, 1, 1);
  gfx_spi_draw_pixels(&lcd->spi, &color, 1);
}

void gc9107_fill_rect(gfx_gc9107_t *lcd, int x, int y, int w, int h, uint16_t color) {
  if (x < 0 || y < 0 || x + w > lcd->width || y + h > lcd->height) {
    return;
  }

  set_addr_window(lcd, x, y, w, h);
  size_t total_pixels = w * h;
  uint16_t buf[64];
  for (int i = 0; i < 64; i++) {
    buf[i] = color;
  }

  while (total_pixels > 0) {
    size_t chunk = (total_pixels > 64) ? 64 : total_pixels;
    gfx_spi_draw_pixels(&lcd->spi, buf, chunk);
    total_pixels -= chunk;
  }
}

void gc9107_backlight_on(gpio_num_t bl_pin) {
  gpio_set_direction(bl_pin, GPIO_MODE_OUTPUT);
  gpio_set_level(bl_pin, 0);
}

void gc9107_backlight_off(gpio_num_t bl_pin) {
  gpio_set_level(bl_pin, 1);
}


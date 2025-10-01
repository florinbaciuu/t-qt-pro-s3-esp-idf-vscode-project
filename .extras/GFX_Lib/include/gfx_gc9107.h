#pragma once

#include "gfx_spi.h"
#include "esp_err.h"
#include "driver/gpio.h"

typedef struct {
    gfx_spi_t spi;
    gpio_num_t rst_pin;
    uint16_t width;
    uint16_t height;
    uint8_t rotation;
    uint8_t ips;
    int16_t x_offset;
    int16_t y_offset;
} gfx_gc9107_t;

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t gc9107_init(gfx_gc9107_t *lcd);
void gc9107_set_rotation(gfx_gc9107_t *lcd, uint8_t rotation);
void gc9107_invert_display(gfx_gc9107_t *lcd, bool invert);
void gc9107_display_on(gfx_gc9107_t *lcd);
void gc9107_display_off(gfx_gc9107_t *lcd);
void gc9107_draw_pixel(gfx_gc9107_t *lcd, int x, int y, uint16_t color);
void gc9107_fill_rect(gfx_gc9107_t *lcd, int x, int y, int w, int h, uint16_t color);
void gc9107_backlight_on(gpio_num_t bl_pin);
void gc9107_backlight_off(gpio_num_t bl_pin);

#ifdef __cplusplus
}
#endif

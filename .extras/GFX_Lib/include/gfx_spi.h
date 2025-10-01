#pragma once

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"

typedef struct {
    spi_device_handle_t handle;
    gpio_num_t dc_pin;
    gpio_num_t cs_pin;
} gfx_spi_t;

esp_err_t gfx_spi_init(gfx_spi_t *spi, 
                       gpio_num_t mosi, 
                       gpio_num_t miso, 
                       gpio_num_t sck, 
                       gpio_num_t cs, 
                       gpio_num_t dc,
                       int clock_speed_hz);

esp_err_t gfx_spi_send_cmd(gfx_spi_t *spi, uint8_t cmd);
esp_err_t gfx_spi_send_data(gfx_spi_t *spi, const uint8_t *data, size_t len);
esp_err_t gfx_spi_draw_pixels(gfx_spi_t *spi, const uint16_t *pixels, size_t len);

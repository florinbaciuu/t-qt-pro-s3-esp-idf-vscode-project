#include "gfx_spi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "gfx_spi";

static inline void dc_command(gfx_spi_t *spi) {
    gpio_set_level(spi->dc_pin, 0);
}

static inline void dc_data(gfx_spi_t *spi) {
    gpio_set_level(spi->dc_pin, 1);
}

static inline void cs_select(gfx_spi_t *spi) {
    gpio_set_level(spi->cs_pin, 0);
}

static inline void cs_deselect(gfx_spi_t *spi) {
    gpio_set_level(spi->cs_pin, 1);
}

esp_err_t gfx_spi_init(gfx_spi_t *spi, gpio_num_t mosi, gpio_num_t miso, gpio_num_t sck, gpio_num_t cs, gpio_num_t dc, int clock_speed_hz)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = mosi,
        .miso_io_num = miso,
        .sclk_io_num = sck,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096
    };

    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to initialize bus: %s", esp_err_to_name(ret));
        return ret;
    }

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = clock_speed_hz,
        .mode = 0,
        .spics_io_num = cs,
        .queue_size = 7,
        .flags = SPI_DEVICE_NO_DUMMY,
    };

    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi->handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add device: %s", esp_err_to_name(ret));
        return ret;
    }

    spi->dc_pin = dc;
    spi->cs_pin = cs;

    gpio_set_direction(dc, GPIO_MODE_OUTPUT);
    gpio_set_direction(cs, GPIO_MODE_OUTPUT);
    cs_deselect(spi);
    dc_data(spi);

    ESP_LOGI(TAG, "SPI init successful");
    return ESP_OK;
}

esp_err_t gfx_spi_send_cmd(gfx_spi_t *spi, uint8_t cmd)
{
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd
    };
    dc_command(spi);
    cs_select(spi);
    esp_err_t ret = spi_device_transmit(spi->handle, &t);
    cs_deselect(spi);
    dc_data(spi);
    return ret;
}

esp_err_t gfx_spi_send_data(gfx_spi_t *spi, const uint8_t *data, size_t len)
{
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data
    };
    dc_data(spi);
    cs_select(spi);
    esp_err_t ret = spi_device_transmit(spi->handle, &t);
    cs_deselect(spi);
    return ret;
}

esp_err_t gfx_spi_draw_pixels(gfx_spi_t *spi, const uint16_t *pixels, size_t len)
{
    spi_transaction_t t = {
        .length = len * 16,
        .tx_buffer = pixels
    };
    dc_data(spi);
    cs_select(spi);
    esp_err_t ret = spi_device_transmit(spi->handle, &t);
    cs_deselect(spi);
    return ret;
}

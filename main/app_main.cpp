/**
 * @file      app_main.cpp.
 */

/*********************
 *      DEFINES
 *********************/
//---------

//---------

/*********************
 *      INCLUDES
 *********************/
extern "C" {
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>

#include "driver/gpio.h"
#include "esp_bootloader_desc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// #include "esp_console.h"
// #include "esp_event.h"
// #include "esp_system.h"
// #include "soc/soc_caps.h"


#include "esp_lcd_gc9107.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"

#include "image_logo.h"
}

#define PIN_MOSI (gpio_num_t)(2)
#define PIN_MISO (gpio_num_t)(-1)
#define PIN_SCK  (gpio_num_t)(3)
#define PIN_CS   (gpio_num_t)(5)
#define PIN_DC   (gpio_num_t)(6)
#define PIN_RST  (gpio_num_t)(1)
#define PIN_BL   (gpio_num_t)(10)

#define LCD_WIDTH  128
#define LCD_HEIGHT 128

// Include that are cpp

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
// ----------------------------------------------------------
// ----------------------------------------------------------

//---------

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
//---------
//---------
//---------

//---------

/*
███████ ██████  ███████ ███████ ██████ ████████  ██████  ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██ ██      
█████   ██████  █████   █████   ██████    ██    ██    ██ ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██      ██ 
██      ██   ██ ███████ ███████ ██   ██   ██     ██████  ███████ 
*/

/*********************
 *  rtos variables
 *********************/
//---------

//---------

// ===== TASK =====
/****************************/
//--------------------------------------

/*
███    ███  █████  ██ ███    ██ 
████  ████ ██   ██ ██ ████   ██ 
██ ████ ██ ███████ ██ ██ ██  ██ 
██  ██  ██ ██   ██ ██ ██  ██ ██ 
██      ██ ██   ██ ██ ██   ████ 
  * This is the main entry point of the application.
  * It initializes the hardware, sets up the display, and starts the LVGL tasks.
  * The application will run indefinitely until the device is powered off or reset.
*/
extern "C" void app_main(void) {
  vTaskDelay(pdMS_TO_TICKS(100));
  esp_log_level_set("*", ESP_LOG_INFO);
  vTaskDelay(pdMS_TO_TICKS(100));

  ESP_LOGI("tft", "Initialize SPI bus");
  spi_bus_config_t bus_config = {
    .mosi_io_num = PIN_MOSI,
    .miso_io_num = PIN_MISO,
    .sclk_io_num = PIN_SCK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .data4_io_num = -1,
    .data5_io_num = -1,
    .data6_io_num = -1,
    .data7_io_num = -1,
    .data_io_default_level = 0,
    .max_transfer_sz = 128 * 128 * sizeof(uint16_t),
    .flags = 0,
    .isr_cpu_id = (esp_intr_cpu_affinity_t)0,
    .intr_flags = 0
  };
  ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO));

  ESP_LOGI("tft", "Install panel IO");
  esp_lcd_panel_io_handle_t io_handle = NULL;
  esp_lcd_panel_io_spi_config_t io_config = {
    .cs_gpio_num = PIN_CS,
    .dc_gpio_num = PIN_DC,
    .spi_mode = 0,                // SPI mode 0
    .pclk_hz = 10 * 1000 * 1000,  // 10 MHz
    .trans_queue_depth = 10,      // câte tranzacții în coadă
    .on_color_trans_done = NULL,  // poți pune callback dacă vrei
    .user_ctx = NULL,
    .lcd_cmd_bits = 8,    // 8 biți comandă
    .lcd_param_bits = 8,  // 8 biți parametri
    .cs_ena_pretrans = 8,
    .cs_ena_posttrans = 8,
    .flags{.dc_high_on_cmd = 0, .dc_low_on_data = 0, .dc_low_on_param = 0, .octal_mode = 0, .quad_mode = 0, .sio_mode = 0, .lsb_first = 0, .cs_high_active = 0}
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle));

  /**
 * Uncomment these lines if use custom initialization commands.
 * The array should be declared as "static const" and positioned outside the function.
 * 
    0x36 = 0xC8 bgr sau 0xC0 rgb intors la 90 grade
 */
  static const gc9107_lcd_init_cmd_t lcd_init_cmds[] = {
    {0xfe, (uint8_t[]){0x00}, 0, 0},
    {0xef, (uint8_t[]){0x00}, 0, 0},
    {0xeb, (uint8_t[]){0x14}, 1, 0},
    {0x84, (uint8_t[]){0x40}, 1, 0},
    {0x85, (uint8_t[]){0xFF}, 1, 0},
    {0x86, (uint8_t[]){0xFF}, 1, 0},
    {0x87, (uint8_t[]){0xFF}, 1, 0},
    {0x88, (uint8_t[]){0x0A}, 1, 0},
    {0x89, (uint8_t[]){0x21}, 1, 0},
    {0x8A, (uint8_t[]){0x00}, 1, 0},
    {0x8B, (uint8_t[]){0x80}, 1, 0},
    {0x8C, (uint8_t[]){0x01}, 1, 0},
    {0x50, (uint8_t[]){0x78}, 1, 0},
    {0x51, (uint8_t[]){0x78}, 1, 0},
    {0x52, (uint8_t[]){0x00}, 1, 0},
    {0x53, (uint8_t[]){0x4A}, 1, 0},
    {0x54, (uint8_t[]){0x00}, 1, 0},
    {0x55, (uint8_t[]){0x4A}, 1, 0},
    {0x3A, (uint8_t[]){0x05}, 1, 0},  // pixel format = 16-bit
    {0x36, (uint8_t[]){0xC8}, 1, 0},  // MADCTL = RGB (schimbi în 0x08 pentru BGR) 0x00 pt RGB
    {0x11, NULL, 0, 120},             // Sleep Out
    {0x21, NULL, 0, 0},               /// inversion OF (dacă vrei efect negativ pune 0x20)
    {0x29, NULL, 0, 100},             // Display ON
  };

  esp_lcd_panel_io_tx_param(io_handle, 0x29, NULL, 0); //echivalent cu {0x29, NULL, 0, 100},  

  ESP_LOGI("tft", "Install GC9107 panel driver");
  esp_lcd_panel_handle_t panel_handle = NULL;
  gc9107_vendor_config_t vendor_config = {
    .init_cmds = lcd_init_cmds,
    .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(gc9107_lcd_init_cmd_t),
  };

  esp_lcd_panel_dev_config_t panel_config = {
    .reset_gpio_num = PIN_RST,
    .rgb_ele_order = LCD_RGB_ENDIAN_BGR,
    .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
    .bits_per_pixel = 16,
    .flags =
      {
        .reset_active_high = 0,
      },
    .vendor_config = &vendor_config,
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_gc9107(io_handle, &panel_config, &panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

  /* GC9107 are regiștri de „column address set” și „row address set” (0x2A și 0x2B).
Unele versiuni de panou au zone „dummy” de 2 pixeli pe X și 1 pe Y. */
  ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, 2, 1));  //special pt GC9107
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));


/* Dacă panoul e pe BGR (MADCTL.BGR = 1) – vizual corect pe ecran BGR */
#define COL_RED    0x07E0   // verde în standard, dar la tine e roșu
#define COL_GREEN  0x001F   // albastru în standard, dar la tine e verde
#define COL_BLUE   0xF800   // roșu în standard, dar la tine e albastru
#define COL_WHITE  0xFFFF
#define COL_BLACK  0x0000
  uint16_t selected_color = COL_RED;  // aici selecteaza culoarea
  static uint16_t color_buf[128 * 128];
  for (int i = 0; i < 128 * 128; i++) {
    color_buf[i] = selected_color;
  }
  esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, 128, 128, color_buf);
  //esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, 128, 128, &gImage_image_logo);
}  // app_main

/********************************************** */

/**********************
 *   END OF FILE
 **********************/

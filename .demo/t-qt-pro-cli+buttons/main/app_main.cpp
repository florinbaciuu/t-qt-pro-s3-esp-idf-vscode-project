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

#include "driver/gpio.h"
#include "esp_bootloader_desc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "argtable3/argtable3.h"
#include "driver/sdmmc_host.h"
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "esp_console.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_vfs_fat.h"
#include "linenoise/linenoise.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "sdmmc_cmd.h"
#include "soc/soc_caps.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "command_line_interface.h"
#include "button.h"
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

/* Console command history can be stored to and loaded from a file.
 * The easiest way to do this is to use FATFS filesystem on top of
 * wear_levelling library.
 */
#if CONFIG_CONSOLE_STORE_HISTORY
#ifdef SDCARD_USE
#else

// Handle of the wear levelling library instance
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

esp_err_t initialize_filesystem() {
  const esp_vfs_fat_mount_config_t config = {
    .format_if_mount_failed = true,
    .max_files = 5,
    .allocation_unit_size = CONFIG_WL_SECTOR_SIZE,
    .disk_status_check_enable = false,
    .use_one_fat = false,
  };
  esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl(MOUNT_PATH, PARTITION_LABEL, &config, &s_wl_handle);
  if (err != ESP_OK) {
    ESP_LOGE("ffat", "Failed to mount FATFS (%s)", esp_err_to_name(err));
    return err;
  }
  ESP_LOGI("ffat", "Mounted FATFS at %s", MOUNT_PATH);
  return ESP_OK;
}
#endif /* #ifdef (SDCARD_USE) */

void init_filesystem() {
#if CONFIG_CONSOLE_STORE_HISTORY
  esp_err_t history_fs_ok = ESP_OK;
#ifdef SDCARD_USE
// This board not have sdcard
#else
  history_fs_ok = initialize_filesystem();
  if (history_fs_ok == ESP_OK) {
    ESP_LOGI("File System", "File system enabled on FFAT");
  } else {
    ESP_LOGW("File System", "Failed to enable file system on FFAT (%s)", esp_err_to_name(history_fs_ok));
  }
#endif /* #ifdef SDCARD_USE */

  if (history_fs_ok == ESP_OK) {
    cli_set_history_path(MOUNT_PATH "/history.txt");
    ESP_LOGI("CLI", "Command history enabled on " MOUNT_PATH);
  } else {
    ESP_LOGW("CLI", "⚠️ Filesystem not mounted, disabling command history");
    cli_set_history_path(NULL);  // fără history
  }
#else
  ESP_LOGI("CONSOLE", "Command history disabled");
#endif /* #if CONFIG_CONSOLE_STORE_HISTORY */
}

#else
// #define HISTORY_PATH NULL
#endif  // CONFIG_CONSOLE_STORE_HISTORY

static void initialize_nvs(void) {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
}

// ----------------------------------------------------------

esp_err_t initialize_eeproom() {
  ESP_LOGI("eeproom", "🔧 Initializing NVS partition 'eeproom'...");

  esp_err_t err = nvs_flash_init_partition("eeproom");
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_LOGW("eeproom", "⚠️ NVS partition is full or version mismatch. Erasing...");
    err = nvs_flash_erase_partition("eeproom");
    if (err != ESP_OK) {
      ESP_LOGE("eeproom", "❌ Failed to erase 'eeproom' partition: %s", esp_err_to_name(err));
      return err;
    }
    err = nvs_flash_init_partition("eeproom");
    if (err != ESP_OK) {
      ESP_LOGE("eeproom", "❌ Failed to re-initialize 'eeproom' after erase: %s", esp_err_to_name(err));
      return err;
    }
  }
  if (err == ESP_OK) {
    ESP_LOGI("eeproom", "✅ NVS partition 'eeproom' initialized successfully");
  } else {
    ESP_LOGE("eeproom", "❌ Failed to initialize NVS: %s", esp_err_to_name(err));
    return err;
  }
  // Deschidem un handle ca să verificăm spațiul
  nvs_handle_t handle;
  err = nvs_open_from_partition("eeproom", "diagnostic", NVS_READWRITE, &handle);
  if (err != ESP_OK) {
    ESP_LOGE("eeproom", "❌ Can't open NVS handle for diagnostics: %s", esp_err_to_name(err));
    return err;
  }
  nvs_close(handle);
  return ESP_OK;
}

//---------

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
//---------

OneButton button;  // global

// ===== CALLBACKS =====

static const char *BUTTON_TAG = "Button";

void click_cb(void) {
  ESP_LOGI(BUTTON_TAG, "Single Click detected!");
}

void double_click_cb(void) {
  ESP_LOGI(BUTTON_TAG, "Double Click detected!");
}

void triple_click_cb(void) {
  ESP_LOGI(BUTTON_TAG, "Triple or multi-click detected!");
}

void long_press_start_cb(void) {
  ESP_LOGI(BUTTON_TAG, "Long Press START");
}

void long_press_stop_cb(void) {
  ESP_LOGI(BUTTON_TAG, "Long Press STOP");
}

void during_long_press_cb(void) {
  ESP_LOGI(BUTTON_TAG, "During Long Press...");
}
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
void button_task(void *pvParameters) {
  (void)pvParameters;
  while (1) {
    button.tick();                  // citește starea butonului și rulează FSM
    vTaskDelay(pdMS_TO_TICKS(10));  // 10 ms
  }
}
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

  esp_bootloader_desc_t bootloader_desc;
  ESP_LOGI("Bootloader", "Details about bootloader:\n");
  esp_rom_printf("\tESP-IDF version from 2nd stage bootloader: %s\n", bootloader_desc.idf_ver);
  esp_rom_printf("\tESP-IDF version from app: %s\n", IDF_VER);

  button = OneButton((gpio_num_t)0, true, true);
  // Setări timpi
  button.setDebounceTicks(50);  // Debounce 50 ms
  button.setClickTicks(400);    // Interval click 400 ms
  button.setPressTicks(800);    // Long press > 800 ms
  // Attach callbacks
  button.attachClick(click_cb);
  button.attachDoubleClick(double_click_cb);
  button.attachMultiClick(triple_click_cb);
  button.attachLongPressStart(long_press_start_cb);
  button.attachDuringLongPress(during_long_press_cb);
  button.attachLongPressStop(long_press_stop_cb);

  // Creează un task separat
  xTaskCreate(button_task, "button_task", 3072, NULL, 10, NULL);  // 3 kb

  // start_resource_monitor();
  heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
  vTaskDelay(pdMS_TO_TICKS(100));
  init_filesystem();
  // initialize_nvs();
  // ESP_ERROR_CHECK(initialize_eeproom());
  vTaskDelay(pdMS_TO_TICKS(100));
  StartCLI();

}  // app_main

/********************************************** */

/**********************
 *   END OF FILE
 **********************/

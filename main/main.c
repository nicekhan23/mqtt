#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi_smart.h"

void app_main(void)
{
    // Initialize NVS (mandatory before Wi-Fi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // Initialize SmartConfig Wi-Fi logic
    ws_init();

    // Your app’s main logic could go here
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

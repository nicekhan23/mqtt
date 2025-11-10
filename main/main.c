#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "econsole.h"
#include "wifi_smart.h"
#include "mqtt.h"
#include "test.h"
#include "ntp.h"

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    configASSERT(ws_init());
    configASSERT(con_init());
    configASSERT(mq_init());
    configASSERT(test_init());
    configASSERT(ntp_init());
}
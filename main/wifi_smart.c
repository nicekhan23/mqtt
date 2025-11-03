#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <esp_event.h>
#include <esp_smartconfig.h>
#include <nvs_flash.h>
#include <string.h>

#include "wifi_smart.h"  // your custom header, if needed

// ======================
// Defines
// ======================
#define CONNECTED_BIT      BIT0
#define ESPTOUCH_DONE_BIT  BIT1

// ======================
// Globals
// ======================
static const char *TAG = "WIFI_SMART";
static EventGroupHandle_t s_wifi_event_group;

// ======================
// Function Declarations
// ======================
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data);
static void smartconfig_example_task(void *param);
static void initialise_wifi(void);

// ======================
// Event Handler
// ======================
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
        return;
    }
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
        return;
    }
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
        return;
    }
    if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        ESP_LOGI(TAG, "SmartConfig: Scan done");
        return;
    }
    if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        ESP_LOGI(TAG, "SmartConfig: Found channel");
        return;
    }
    if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(TAG, "SmartConfig: Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        bzero(&wifi_config, sizeof(wifi_config_t));

        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));

        ESP_LOGI(TAG, "SSID: %s", (char *)wifi_config.sta.ssid);
        ESP_LOGI(TAG, "PASSWORD: %s", (char *)wifi_config.sta.password);

        if (evt->type == SC_TYPE_ESPTOUCH_V2) {
            uint8_t rvd_data[33] = {0};
            ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)));
            ESP_LOGI(TAG, "RVD_DATA:");
            for (int i = 0; i < 33; i++) {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_connect());
    }
    if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
        return;
    }
}

// ======================
// SmartConfig Task
// ======================
static void smartconfig_example_task(void *param)
{
    EventBits_t uxBits;
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));

    while (1) {
        uxBits = xEventGroupWaitBits(s_wifi_event_group,
                                     CONNECTED_BIT | ESPTOUCH_DONE_BIT,
                                     true, false, portMAX_DELAY);
        if (uxBits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to AP");
        }
        if (uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "SmartConfig complete");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}

// ======================
// Wi-Fi Initialization
// ======================
static void initialise_wifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

// ======================
// Public API
// ======================
BaseType_t ws_init()
{
    ESP_LOGI(TAG, "Initializing Wi-Fi SmartConfig...");
    ESP_ERROR_CHECK(nvs_flash_init());
    initialise_wifi();
    return pdPASS;
}

BaseType_t ws_deinit()
{
    ESP_LOGI(TAG, "Deinitializing Wi-Fi...");
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());
    return pdPASS;
}

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_sntp.h"
#include "lwip/ip_addr.h"
#include "esp_netif_sntp.h"
#include "esp_wifi.h"
#include "ntp.h"
#include <time.h>

#define LOG_LEVEL_LOCAL ESP_LOG_DEBUG

static const char* TAG = "NTP";

void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG, "NTP time synchronization event");
}

static void obtain_time(void) {
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    config.start = false;
    config.server_from_dhcp = true;
    config.renew_servers_after_new_IP = true;
    config.index_of_first_server = 1;
    config.ip_event_to_renew = IP_EVENT_STA_GOT_IP;
    config.sync_cb = time_sync_notification_cb;
    config.smooth_sync = true;
    esp_netif_sntp_init(&config);
    esp_netif_sntp_start();
    
    time_t now;
    struct tm timeinfo = { 0 };
    const int retry_count = 20;
    int retry = 0;
    while (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(2000)) == ESP_ERR_TIMEOUT && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/20)", retry);
    }
    time(&now);
    localtime_r(&now, &timeinfo);

    printf("Current time: %s \n", asctime(&timeinfo));

    if (retry == retry_count) {
        ESP_LOGE(TAG, "Failed to obtain time from NTP server");
    } else {
        ESP_LOGI(TAG, "Time obtained from NTP server: %s", asctime(&timeinfo));
    }
}

BaseType_t ntp_init() {
    esp_log_level_set(TAG,LOG_LEVEL_LOCAL);
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    obtain_time();
    return pdPASS;
}

BaseType_t ntp_deinit(void){
    return pdPASS;
}

BaseType_t ntp_get_time(char *timezone, struct tm *timeinfo){
    if (timezone == NULL || timeinfo == NULL) {
         timezone = "UTC";
    }
    time_t now;
    time(&now);
    setenv("TZ", timezone, 1);
    tzset();
    localtime_r(&now, &timeinfo);
    ESP_LOGD(TAG, "NTP time fetched: %s", asctime(&timeinfo));
    return pdPASS;
}
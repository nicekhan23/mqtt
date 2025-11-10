#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "test.h"
#include "mqtt.h"

#define LOG_LEVEL_LOCAL ESP_LOG_DEBUG
#define TOPIC "/ptemik/darkhan"

static const char* TAG = "TEST";
static TaskHandle_t tsk_handle;

static void tsk_test(void *p){
    char *mac = (char *)mqtt_get_mac_string();
    int i = 0;

    for(;;) {
        while (mq_is_connected()) {
        char msg[64];
        ESP_LOGI(TAG, "Publishing message from device %s, count: %d", mac, i);
        snprintf(msg, sizeof(msg), "{\"hello\": \" from %s, i:%d\"}", mac, i++);
        mq_send(TOPIC, msg);
        vTaskDelay(pdMS_TO_TICKS(500));
        }
    } 
}

BaseType_t test_init() {
 esp_log_level_set(TAG,LOG_LEVEL_LOCAL); 
 return xTaskCreate(tsk_test, "test", 4096, NULL, uxTaskPriorityGet(NULL), &tsk_handle);
}
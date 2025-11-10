#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "mqtt.h"
#include "esp_mac.h"

#define LOG_LEVEL_LOCAL ESP_LOG_DEBUG

static const char* TAG = "MQTT";

static esp_mqtt_client_handle_t client_id = NULL;

static char mac_string[16];

static bool is_connected = false;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        is_connected = true;
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        is_connected = false;
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGE(TAG, "reported from esp-tls: %d", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGE(TAG, "reported from tls stack: %d", event->error_handle->esp_tls_stack_err);
            ESP_LOGE(TAG, "captured as transport's socket errno: %d",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

const char *mqtt_get_mac_string(){
    return mac_string;
}

bool mq_is_connected(){
    return is_connected;
}

BaseType_t mq_init(){
    esp_log_level_set(TAG,LOG_LEVEL_LOCAL);
    mac_t mac_addr;
    bzero(&mac_addr, sizeof(mac_t));
    esp_efuse_mac_get_default(mac_addr.as_bytes);
    snprintf(mac_string, sizeof(mac_string), "%0llx6", mac_addr.as_long);
    printf("Device MAC address: %s\n", mac_string);
    printf("MQTT Broker URI: %s\n", CONFIG_MQTT_BROKER_URI);
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_MQTT_BROKER_URI,
        .credentials = {
            .username = CONFIG_MQTT_USERNAME,
            .authentication = {
                .password = CONFIG_MQTT_PASSWORD,
            }
        }
    };

    client_id = esp_mqtt_client_init(&mqtt_cfg);
    configASSERT(client_id != NULL);

    return (esp_mqtt_client_register_event(client_id, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL) == ESP_OK) &&
           (esp_mqtt_client_start(client_id) == ESP_OK) ? pdPASS : pdFAIL;
}

BaseType_t mq_deinit(){
    return pdPASS;
}



BaseType_t mq_send(char *topic, char *message){
    if (topic == NULL || message == NULL) {
        ESP_LOGE(TAG, "Topic or message is NULL");
        return pdFAIL;
    }

    int msg_id = esp_mqtt_client_publish(client_id, topic, message, 0, 1, 0);
    ESP_LOGI(TAG, "Message published, msg_id=%d", msg_id);

    return pdPASS;
}
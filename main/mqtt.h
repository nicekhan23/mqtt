#ifndef MQTT_H
#define MQTT_H

#include "freertos/FreeRTOS.h"
#include <stdbool.h>

typedef union {
    uint8_t as_bytes[8];
    uint64_t as_long;
} mac_t;

BaseType_t mq_init();
BaseType_t mq_deinit();
BaseType_t mq_send(char *topic, char *message);
bool mq_is_connected();
const char *mqtt_get_mac_string();

#endif /* MQTT_H */
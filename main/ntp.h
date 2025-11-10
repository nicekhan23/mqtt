#ifndef NTP_H
#define NTP_H

#include "freertos/FreeRTOS.h"
#include <time.h>

BaseType_t ntp_init(void);
BaseType_t ntp_deinit(void);
BaseType_t ntp_get_time(char *timezone, struct tm *time);

#endif /* NTP_H */
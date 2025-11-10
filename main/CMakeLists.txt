idf_component_register(SRCS "wifi_smart.c"
                            "main.c"
                            "mqtt.c"
                            "test.c"
                    PRIV_REQUIRES esp_wifi wpa_supplicant nvs_flash econsole mqtt esp_driver_gpio
                    INCLUDE_DIRS ".")

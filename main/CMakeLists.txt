idf_component_register(SRCS "wifi_smart.c"
                            "main.c"
                    PRIV_REQUIRES esp_wifi wpa_supplicant nvs_flash
                    INCLUDE_DIRS ".")

idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES dht11 ssd1306 ir_sensor status_led nvs_flash esp_app_format)

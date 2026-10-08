idf_component_register(SRCS "ssd1306.c" "font8x8.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES esp_driver_i2c)

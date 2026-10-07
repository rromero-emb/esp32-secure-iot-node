# Session 1: the implementation will add `REQUIRES esp_driver_i2c`.
idf_component_register(SRCS "ssd1306.c"
                       INCLUDE_DIRS "include")

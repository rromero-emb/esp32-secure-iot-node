# Session 1: the implementation will add `REQUIRES esp_driver_rmt`.
idf_component_register(SRCS "dht11.c"
                       INCLUDE_DIRS "include")

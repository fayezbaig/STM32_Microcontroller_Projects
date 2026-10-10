idf_component_register(
    SRCS "main.c" "ble_server.c" "uart_rx.c" "sensor_frame.c"
    INCLUDE_DIRS "."
    REQUIRES bt nvs_flash driver
)

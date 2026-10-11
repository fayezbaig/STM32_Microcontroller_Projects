idf_component_register(
    SRCS "main.c" "tracker.c" "altitude.c" "bmp280.c" "button.c" "wifi_ap.c" "web_server.c"
    INCLUDE_DIRS "."
    EMBED_FILES "index.html"
    REQUIRES driver esp_wifi esp_netif esp_event esp_http_server esp_timer nvs_flash
)

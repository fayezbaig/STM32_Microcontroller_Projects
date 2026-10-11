/*
 * web_server.c - HTTP server for the phone dashboard
 */
#include "web_server.h"

#include <stdio.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "tracker.h"

#define TAG "web"

/* index.html is linked into the firmware (EMBED_FILES in CMakeLists.txt) */
extern const char index_html_start[] asm("_binary_index_html_start");
extern const char index_html_end[]   asm("_binary_index_html_end");

static esp_err_t root_get(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    return httpd_resp_send(req, index_html_start, index_html_end - index_html_start);
}

static esp_err_t state_get(httpd_req_t *req)
{
    tracker_state_t s;
    tracker_get_state(&s);

    char json[320];
    const int n = snprintf(json, sizeof json,
        "{\"ok\":%s,\"floor\":%d,\"height\":%.2f,\"speed\":%.2f,"
        "\"motion\":\"%s\",\"pressure\":%.2f,\"temperature\":%.1f,"
        "\"floorHeight\":%.2f,\"uptime\":%lu}",
        s.sensor_ok ? "true" : "false", s.floor, s.height_m, s.speed_mps,
        altitude_motion_str(s.motion), s.pressure_hpa, s.temperature_c,
        s.floor_height_m, (unsigned long)s.uptime_s);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    return httpd_resp_send(req, json, n);
}

static esp_err_t zero_post(httpd_req_t *req)
{
    tracker_request_zero();
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_sendstr(req, "{\"ok\":true}");
}

esp_err_t web_server_start(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    esp_err_t err = httpd_start(&server, &config);
    if (err != ESP_OK) return err;

    const httpd_uri_t routes[] = {
        { .uri = "/",          .method = HTTP_GET,  .handler = root_get,  .user_ctx = NULL },
        { .uri = "/api/state", .method = HTTP_GET,  .handler = state_get, .user_ctx = NULL },
        { .uri = "/api/zero",  .method = HTTP_POST, .handler = zero_post, .user_ctx = NULL },
    };
    for (size_t i = 0; i < sizeof routes / sizeof routes[0]; i++) {
        if ((err = httpd_register_uri_handler(server, &routes[i])) != ESP_OK) return err;
    }

    ESP_LOGI(TAG, "web server running");
    return ESP_OK;
}

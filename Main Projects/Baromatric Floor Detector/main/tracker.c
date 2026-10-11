/*
 * tracker.c - FreeRTOS tasks: sensor sampling and altitude processing
 */
#include "tracker.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"

#include "bmp280.h"

#define TAG                 "tracker"
#define SAMPLE_PERIOD_MS    50          /* 20 Hz */
#define QUEUE_LEN           20          /* 1 s of buffering */
#define WARMUP_SAMPLES      60          /* 3 s for the Kalman filter to settle */

typedef struct {
    int64_t t_us;
    float   pressure_pa;
    float   temperature_c;
} sample_t;

static QueueHandle_t     s_queue;
static SemaphoreHandle_t s_state_mutex;
static TaskHandle_t      s_tracker_task;
static tracker_state_t   s_state;

/* ------------------------------------------------------------------ */
/* Task 1: read the sensor at a fixed rate                             */
/* ------------------------------------------------------------------ */
static void sensor_task(void *arg)
{
    (void)arg;
    TickType_t last_wake = xTaskGetTickCount();
    uint32_t   errors = 0;

    for (;;) {
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(SAMPLE_PERIOD_MS));   /* no drift */

        bmp280_reading_t r;
        if (bmp280_read(&r) != ESP_OK) {
            if (++errors % 20 == 1) ESP_LOGW(TAG, "sensor read failed (%lu)", (unsigned long)errors);
            continue;
        }

        const sample_t s = {
            .t_us          = esp_timer_get_time(),
            .pressure_pa   = r.pressure_pa,
            .temperature_c = r.temperature_c,
        };
        if (xQueueSend(s_queue, &s, 0) != pdTRUE) {
            ESP_LOGW(TAG, "queue full - processing too slow");
        }
    }
}

/* ------------------------------------------------------------------ */
/* Task 2: filter, detect floors, publish state                        */
/* ------------------------------------------------------------------ */
static void tracker_task(void *arg)
{
    (void)arg;
    altitude_ctx_t ctx;
    altitude_init(&ctx, (float)CONFIG_FLOOR_HEIGHT_CM / 100.0f);

    int64_t  last_t_us = 0;
    uint32_t n = 0;
    uint32_t last_log_s = 0;

    for (;;) {
        sample_t s;
        if (xQueueReceive(s_queue, &s, pdMS_TO_TICKS(1000)) != pdTRUE) {
            xSemaphoreTake(s_state_mutex, portMAX_DELAY);
            s_state.sensor_ok = false;
            xSemaphoreGive(s_state_mutex);
            continue;
        }

        const float dt = last_t_us ? (float)(s.t_us - last_t_us) / 1e6f : 0.05f;
        last_t_us = s.t_us;
        altitude_update(&ctx, s.pressure_pa, dt);
        n++;

        /* Automatic zero once the filter has settled after power-on,
         * and on request (button ISR or web page -> task notification) */
        if (n == WARMUP_SAMPLES || ulTaskNotifyTake(pdTRUE, 0) > 0) {
            altitude_zero(&ctx);
            ESP_LOGI(TAG, "ground floor set at %.2f hPa", s.pressure_pa / 100.0f);
        }

        xSemaphoreTake(s_state_mutex, portMAX_DELAY);
        s_state.sensor_ok      = true;
        s_state.floor          = ctx.floor;
        s_state.height_m       = altitude_relative(&ctx);
        s_state.speed_mps      = ctx.v_smooth;
        s_state.motion         = ctx.motion;
        s_state.pressure_hpa   = s.pressure_pa / 100.0f;
        s_state.temperature_c  = s.temperature_c;
        s_state.floor_height_m = ctx.floor_height_m;
        s_state.uptime_s       = (uint32_t)(s.t_us / 1000000);
        s_state.samples        = n;
        xSemaphoreGive(s_state_mutex);

        if (s_state.uptime_s != last_log_s) {                /* 1 Hz log */
            last_log_s = s_state.uptime_s;
            ESP_LOGI(TAG, "floor %d | %+.2f m | %+.2f m/s | %s | %.2f hPa | %.1f C",
                     ctx.floor, altitude_relative(&ctx), ctx.v_smooth,
                     altitude_motion_str(ctx.motion), s.pressure_pa / 100.0f, s.temperature_c);
        }
    }
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */
esp_err_t tracker_start(void)
{
    s_queue       = xQueueCreate(QUEUE_LEN, sizeof(sample_t));
    s_state_mutex = xSemaphoreCreateMutex();
    if (!s_queue || !s_state_mutex) return ESP_ERR_NO_MEM;

    esp_err_t err = bmp280_init(CONFIG_BMP280_SDA_GPIO, CONFIG_BMP280_SCL_GPIO,
                                (uint8_t)CONFIG_BMP280_I2C_ADDR);
    if (err != ESP_OK) return err;

    if (xTaskCreate(tracker_task, "tracker", 4096, NULL, 5, &s_tracker_task) != pdPASS) return ESP_ERR_NO_MEM;
    if (xTaskCreate(sensor_task,  "sensor",  4096, NULL, 6, NULL)            != pdPASS) return ESP_ERR_NO_MEM;
    return ESP_OK;
}

void tracker_get_state(tracker_state_t *out)
{
    xSemaphoreTake(s_state_mutex, portMAX_DELAY);
    memcpy(out, &s_state, sizeof *out);
    xSemaphoreGive(s_state_mutex);
}

void tracker_request_zero(void)
{
    if (s_tracker_task) xTaskNotifyGive(s_tracker_task);
}

void tracker_request_zero_from_isr(void)
{
    BaseType_t woken = pdFALSE;
    if (s_tracker_task) vTaskNotifyGiveFromISR(s_tracker_task, &woken);
    portYIELD_FROM_ISR(woken);
}

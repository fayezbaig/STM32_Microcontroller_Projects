/*
 * tracker.h - FreeRTOS task structure of the floor tracker
 *
 *   sensor_task  (prio 6, 20 Hz)  BMP280 -> sample queue
 *   tracker_task (prio 5)         queue -> Kalman / floor detection
 *                                 -> shared state (mutex protected)
 *   web server / button           read state, request "zero"
 *                                 (task notification to tracker_task)
 */
#ifndef TRACKER_H
#define TRACKER_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "altitude.h"

typedef struct {
    bool     sensor_ok;
    int      floor;
    float    height_m;        /* relative to floor 0 */
    float    speed_mps;       /* + = up */
    motion_t motion;
    float    pressure_hpa;
    float    temperature_c;
    float    floor_height_m;
    uint32_t uptime_s;
    uint32_t samples;
} tracker_state_t;

esp_err_t tracker_start(void);

/* Thread-safe copy of the latest state */
void tracker_get_state(tracker_state_t *out);

/* "Set ground floor here" - from a task or from an ISR */
void tracker_request_zero(void);
void tracker_request_zero_from_isr(void);

#endif /* TRACKER_H */

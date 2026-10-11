/*
 * altitude.h - pressure -> height, Kalman filter, floor & motion detection
 *
 * Pure C (only <math.h>), no ESP-IDF dependency, so the whole algorithm
 * can be unit-tested on a PC with simulated data.
 *
 * Pipeline per sample:
 *   pressure [Pa] --barometric formula--> raw height [m]
 *                 --Kalman filter-------> height + vertical speed
 *                 --hysteresis----------> floor number
 *                 --speed---------------> still / stairs / elevator
 *                 --drift compensation--> keeps floors aligned over hours
 */
#ifndef ALTITUDE_H
#define ALTITUDE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    MOTION_STILL = 0,
    MOTION_STAIRS_UP,
    MOTION_STAIRS_DOWN,
    MOTION_ELEVATOR_UP,
    MOTION_ELEVATOR_DOWN,
} motion_t;

typedef struct {
    /* ---- configuration (set by altitude_init, may be tuned) ---- */
    float floor_height_m;    /* height of one storey, e.g. 3.0 m              */
    float hysteresis_m;      /* margin before switching floor, e.g. 0.5 m     */
    float still_speed_mps;   /* |v| below this = not moving                   */
    float elevator_speed_mps;/* peak |v| above this = elevator, else stairs    */
    float sigma_acc;         /* Kalman process noise: vertical accel std [m/s2] */
    float sigma_meas;        /* Kalman measurement noise: height std [m]       */
    float drift_delay_s;     /* stand still this long before drift correction */
    float drift_tau_s;       /* time constant of the drift correction         */
    float speed_tau_s;       /* smoothing of the speed used for motion detection */

    /* ---- Kalman state: x = [height, vertical speed] ---- */
    float h;                 /* absolute ISA height [m] */
    float v;                 /* vertical speed [m/s], + = up */
    float P[2][2];           /* covariance */
    bool  initialised;

    /* ---- detection state ---- */
    float    ref_h;          /* absolute height of floor 0 */
    int      floor;
    motion_t motion;
    float    still_time_s;
    float    move_peak_mps;  /* highest |v| in the current movement */
    float    v_smooth;       /* low-pass speed for still/moving decisions */
} altitude_ctx_t;

/* Set defaults; floor_height_m e.g. 3.0 */
void altitude_init(altitude_ctx_t *c, float floor_height_m);

/* International barometric formula (ISA), 0 m at 1013.25 hPa */
float altitude_from_pressure(float pressure_pa);

/* Feed one pressure sample; dt = seconds since previous sample */
void altitude_update(altitude_ctx_t *c, float pressure_pa, float dt);

/* "I am on floor 0 now": current height becomes the reference */
void altitude_zero(altitude_ctx_t *c);

/* Height relative to floor 0 [m] */
float altitude_relative(const altitude_ctx_t *c);

const char *altitude_motion_str(motion_t m);

#endif /* ALTITUDE_H */

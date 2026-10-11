/*
 * altitude.c - pressure -> height, Kalman filter, floor & motion detection
 */
#include "altitude.h"

#include <math.h>

#define SEA_LEVEL_PA   101325.0f

void altitude_init(altitude_ctx_t *c, float floor_height_m)
{
    c->floor_height_m     = floor_height_m;
    c->hysteresis_m       = 0.5f;
    c->still_speed_mps    = 0.08f;
    c->elevator_speed_mps = 0.80f;  /* stairs ~0.2-0.6 m/s, elevators ~1-2.5 m/s */
    c->sigma_acc          = 0.30f;
    c->sigma_meas         = 0.30f;
    c->drift_delay_s      = 20.0f;
    c->drift_tau_s        = 120.0f;
    c->speed_tau_s        = 1.5f;

    c->h = 0.0f;
    c->v = 0.0f;
    c->P[0][0] = 10.0f; c->P[0][1] = 0.0f;
    c->P[1][0] = 0.0f;  c->P[1][1] = 1.0f;
    c->initialised = false;

    c->ref_h         = 0.0f;
    c->floor         = 0;
    c->motion        = MOTION_STILL;
    c->still_time_s  = 0.0f;
    c->move_peak_mps = 0.0f;
    c->v_smooth      = 0.0f;
}

float altitude_from_pressure(float pressure_pa)
{
    /* h = 44330 * (1 - (p/p0)^(1/5.255)) ; about 8.4 cm per Pa near sea level */
    return 44330.0f * (1.0f - powf(pressure_pa / SEA_LEVEL_PA, 0.1902949f));
}

float altitude_relative(const altitude_ctx_t *c)
{
    return c->h - c->ref_h;
}

void altitude_zero(altitude_ctx_t *c)
{
    c->ref_h = c->h;
    c->floor = 0;
}

/* ------------------------------------------------------------------ */
/* Kalman filter, constant-velocity model                              */
/*   x = [h, v],  F = [1 dt; 0 1],  H = [1 0]                          */
/*   Q = sigma_acc^2 * [dt^4/4 dt^3/2; dt^3/2 dt^2],  R = sigma_meas^2  */
/* ------------------------------------------------------------------ */
static void kalman_step(altitude_ctx_t *c, float z, float dt)
{
    float (*P)[2] = c->P;
    const float q  = c->sigma_acc * c->sigma_acc;
    const float dt2 = dt * dt;

    /* --- predict --- */
    c->h += c->v * dt;

    const float p00 = P[0][0] + dt * (P[0][1] + P[1][0]) + dt2 * P[1][1] + q * dt2 * dt2 * 0.25f;
    const float p01 = P[0][1] + dt * P[1][1]                              + q * dt2 * dt * 0.5f;
    const float p10 = P[1][0] + dt * P[1][1]                              + q * dt2 * dt * 0.5f;
    const float p11 = P[1][1]                                             + q * dt2;

    /* --- update with measured height z --- */
    const float y  = z - c->h;                         /* innovation */
    const float s  = p00 + c->sigma_meas * c->sigma_meas;
    const float k0 = p00 / s;
    const float k1 = p10 / s;

    c->h += k0 * y;
    c->v += k1 * y;

    P[0][0] = (1.0f - k0) * p00;
    P[0][1] = (1.0f - k0) * p01;
    P[1][0] = p10 - k1 * p00;
    P[1][1] = p11 - k1 * p01;
}

/* Switch floor only when clearly inside the band of the new floor:
 * e.g. 3.0 m floors, 0.5 m hysteresis -> must be within 1.0 m of the
 * new floor's level. Standing on a stair landing (1.5 m) never flickers. */
static void detect_floor(altitude_ctx_t *c)
{
    const float rel     = altitude_relative(c);
    const int   nearest = (int)lroundf(rel / c->floor_height_m);

    if (nearest != c->floor) {
        const float dist = fabsf(rel - (float)nearest * c->floor_height_m);
        if (dist < (c->floor_height_m * 0.5f - c->hysteresis_m)) {
            c->floor = nearest;
        }
    }
}

/* Decisions use a smoothed speed (~1.5 s low-pass of the Kalman speed):
 * with a noisy sensor the raw estimate keeps poking above the "still"
 * threshold, which would stop the drift compensation from ever running.
 * A movement is classified by its PEAK speed, so an elevator that is
 * still accelerating is not reported as "stairs". */
static void detect_motion(altitude_ctx_t *c, float dt)
{
    c->v_smooth += (c->v - c->v_smooth) * (dt / (c->speed_tau_s + dt));
    const float speed = fabsf(c->v_smooth);

    if (speed < c->still_speed_mps) {
        c->motion = MOTION_STILL;
        c->still_time_s += dt;
        if (c->still_time_s > 2.0f) c->move_peak_mps = 0.0f;   /* movement over */
        return;
    }

    c->still_time_s = 0.0f;
    if (speed > c->move_peak_mps) c->move_peak_mps = speed;

    const bool up       = c->v_smooth > 0.0f;
    const bool elevator = c->move_peak_mps >= c->elevator_speed_mps;

    if (elevator) c->motion = up ? MOTION_ELEVATOR_UP : MOTION_ELEVATOR_DOWN;
    else          c->motion = up ? MOTION_STAIRS_UP   : MOTION_STAIRS_DOWN;
}

/* Weather slowly changes air pressure (typically 0.1-0.3 hPa/h, i.e.
 * 1-2.5 m of apparent height per hour). Nobody moves vertically that
 * slowly inside a building, so while standing still on a floor the
 * reference is gently pulled until the floor level matches again. */
static void compensate_drift(altitude_ctx_t *c, float dt)
{
    if (c->motion != MOTION_STILL || c->still_time_s < c->drift_delay_s) return;

    const float error = altitude_relative(c) - (float)c->floor * c->floor_height_m;
    if (fabsf(error) > c->floor_height_m * 0.5f - c->hysteresis_m) return;  /* not sure */

    c->ref_h += error * (dt / c->drift_tau_s);
}

void altitude_update(altitude_ctx_t *c, float pressure_pa, float dt)
{
    const float z = altitude_from_pressure(pressure_pa);

    if (!c->initialised) {
        c->h = z;
        c->v = 0.0f;
        c->ref_h = z;
        c->initialised = true;
        return;
    }
    if (dt <= 0.0f || dt > 1.0f) dt = 0.05f;   /* guard against timing glitches */

    kalman_step(c, z, dt);
    detect_motion(c, dt);
    detect_floor(c);
    compensate_drift(c, dt);
}

const char *altitude_motion_str(motion_t m)
{
    switch (m) {
    case MOTION_STAIRS_UP:     return "Stairs up";
    case MOTION_STAIRS_DOWN:   return "Stairs down";
    case MOTION_ELEVATOR_UP:   return "Elevator up";
    case MOTION_ELEVATOR_DOWN: return "Elevator down";
    default:                   return "Still";
    }
}

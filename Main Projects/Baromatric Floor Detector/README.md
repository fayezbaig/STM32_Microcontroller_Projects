# Barometric Floor Tracker

*ESP32 · ESP-IDF · FreeRTOS · BMP280 · Kalman filter · WiFi web dashboard*

This project knows which floor of a building you're on, using only air pressure.

Air pressure drops by about 12 Pa for every metre you go up. The BMP280 is sensitive enough to notice the
difference between two floors. The ESP32 turns that tiny pressure change into a height, works out the
current floor, and recognises whether you took the **stairs or the elevator**. Everything appears live on
your phone: the ESP32 creates its own WiFi network, so it works anywhere in a building with no router.

## Features

- **Floor detection** with hysteresis, so the number never flickers on a stair landing between floors
- **A Kalman filter** that estimates height *and* vertical speed from noisy pressure readings
- **Stairs vs elevator** classification from the peak vertical speed of each movement
- **Weather drift compensation:** outdoor air pressure changes over hours would shift the reading by
  metres, so while you stand still the reference slowly re-aligns to the current floor
- **A FreeRTOS design:** separate sensor and processing tasks connected by a queue, a mutex-protected
  shared state, and an ISR-to-task notification for the button
- **A live phone dashboard:** floor, height, speed, pressure, temperature and a 2-minute height graph
- **A host-side simulation test** that checks the algorithm on a PC without any hardware

## Architecture

```
 BMP280 ──I2C──> sensor_task ──queue──> tracker_task ──mutex──> shared state ──> HTTP server ──WiFi──> phone
                 (20 Hz, prio 6)        (prio 5)                                  /api/state
                                        Kalman filter                             /api/zero
                                        floor + motion detection
                                        drift compensation
 BOOT button ──ISR──> task notification ──> tracker_task ("set ground floor here")
```

## Files

| File | Purpose |
|---|---|
| `main/altitude.c/.h` | Pressure-to-height conversion, Kalman filter, floor and motion detection, drift compensation (pure C, testable on a PC) |
| `main/bmp280.c/.h` | BMP280 driver (ESP-IDF I2C master API), datasheet compensation with 1/256 Pa resolution |
| `main/tracker.c/.h` | FreeRTOS tasks, queue, mutex and task notifications |
| `main/button.c/.h` | BOOT button interrupt with debounce |
| `main/wifi_ap.c/.h` | ESP32 WiFi access point |
| `main/web_server.c/.h` | HTTP server: dashboard page and JSON API |
| `main/index.html` | Phone dashboard, embedded in the firmware |
| `main/Kconfig.projbuild` | Settings: WiFi name and password, pins, I2C address, floor height |
| `test/sim_test.c` | Simulated building trip with noise and weather drift, returns PASS/FAIL |

## Wiring

| BMP280 | ESP32 DevKit |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SCL | GPIO 22 |
| SDA | GPIO 21 |

Most modules use I2C address **0x76**. If yours uses 0x77, change it in `menuconfig`.

## How to run

**You need:** ESP-IDF v5.2 or newer, an ESP32 DevKit, a BMP280 module and a phone.

### 1. Build and flash
```
idf.py set-target esp32
idf.py -p <PORT> flash monitor
```
Optional: `idf.py menuconfig`, then **Floor Tracker**, to change the WiFi name and password, the pins,
or the floor height (default 3.0 m).

### 2. Connect your phone
1. Join the WiFi network **FloorTracker** (password `floor1234`).
2. Open **http://192.168.4.1** in the browser.

### 3. Use it
1. Stand still for 3 seconds after power-on. That spot becomes floor **G** (ground).
2. Walk up the stairs or take the elevator, and watch the floor number and the graph change.
3. To set a new ground floor, press **"Set ground floor here"** on the page, or the **BOOT** button on the ESP32.

### Test the algorithm without hardware
```
cd test
make run
```

## How it works

1. **Pressure to height:** the international barometric formula, h = 44330 · (1 − (p / 1013.25 hPa)^0.19029).
   Near the ground, 1 Pa corresponds to about 8.4 cm.
2. **Kalman filter:** a constant-velocity model with state [height, vertical speed], updated 20 times per
   second. It removes sensor noise and also gives the vertical speed.
3. **Floor detection:** the floor only changes when the height is within 1.0 m of the new floor's level
   (with 3 m floors and 0.5 m hysteresis), so standing on a landing between floors doesn't flicker.
4. **Stairs vs elevator:** walking up stairs gives about 0.2–0.6 m/s vertical speed, elevators about 1–2.5 m/s.
   Each movement is classified by its peak speed, with the threshold at 0.8 m/s.
5. **Drift compensation:** weather typically changes pressure by 0.1–0.3 hPa per hour, which would look like
   1–2.5 m of height. People don't move vertically that slowly indoors, so after 20 s of standing still the
   reference is pulled gently (2-minute time constant) back to the current floor's level.

## Limitations

- **Height is relative, not absolute.** The tracker counts floors from the point you set as ground.
- **Assumes equal floor heights.** Buildings with a tall ground floor need a different `FLOOR_HEIGHT_CM` or per-floor values.
- **Pressure jumps from doors and ventilation.** Opening a door or a strong air-conditioning system can cause brief jumps; the Kalman filter smooths most of them.
- **Stairs vs elevator is a heuristic.** A slow residential elevator (below 0.8 m/s) can be reported as stairs.

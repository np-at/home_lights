# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

ESP32 (Arduino framework, PlatformIO) firmware for a "hexlight": 14 hexagon panels × 35 WS2812B LEDs (490 total, data on GPIO 4), controlled over MQTT and updated over the air (OTA).

## Commands

```sh
pio run                              # build both envs (light2_ota, light1_ota)
pio run -e light1_ota                # build one env
pio run -e light1_ota -t upload      # OTA flash via espota (retry if it dies partway; WiFi is flaky)
nc 192.168.1.253 23                  # RemoteDebug telnet console (DEBUG() output, `reset` reboots)
```

The physical hexlight is **`houselet_light1` at 192.168.1.253 → always use `light1_ota`**. In `secrets.json`, `light2_ota`'s `UPLOAD_PORT` also points at .253, which is out of date. Flashing light2 there would change the device's MQTT identity and OTA password.

Testing: `c++ -std=c++17 -Isrc test/host/geometry_check.cpp -o /tmp/geo && /tmp/geo` checks the panel geometry on the host. `test/test_diff/test_dummy.cpp` is an on-device googletest placeholder.

## Secrets / build flags

`add_secrets.py` (a `pre:` extra_script) reads `secrets.json[<PIOENV>]` and turns `BUILD_FLAGS` into `-DKEY=value` for WiFi, MQTT and OTA credentials. It also sets `UPLOAD_FLAGS`, `UPLOAD_PORT` and `UPLOAD_PROTOCOL` for espota.
- `secrets.json` is gitignored and untracked. `secrets.example.json` is the template.
- The script prints every secret it loads, so filter build output (`grep -v '^secrets:'`) before showing it.

String config arrives as bare tokens (e.g. `-DSENSORNAME="houselet_light1"`) and is turned into strings with `STR()`. Strings are built at compile time by concatenation, e.g. `STR(LIGHT_STATE_TOPIC) "/availability"`.

Feature flags:
- `DEBUGENABLE` / `DEBUG_ENABLE`: RemoteDebug telnet logging. Both are defined for both envs.
- `HA_DISCOVERY`: Home Assistant MQTT device discovery, a retained availability topic, last-will `offline`, and `color_mode` in state messages. Only `light1_ota` turns it on.
- `HA_DISCOVERY_PREFIX` (default `homeassistant`).
- `USE_SERIAL`.

## Dependency gotchas

- **RemoteDebug**: use the patched copy in `lib/RemoteDebug`. Do *not* add `joaolopesf/RemoteDebug` to `lib_deps`; the registry 3.0.5 fails on ESP32 core 2.x (`hwcrypto/sha.h`). Its `begin()` takes a `String*`; pass a `static String`, never a cast `const char*`.
- **FastLED is pinned to 3.6.0.** Newer versions move helpers into `fl::` and break `breathe.h`.
- Source files use CRLF line endings. Keep them CRLF when editing, so diffs don't touch every line.

## Architecture

- Almost everything is header-only and pulled into `main.cpp`'s single translation unit. `include/globals.h` holds `static` globals, which are per-TU:
  - `g_LEDS`
  - `currentState` / `desiredState` (`State`)
  - the RemoteDebug `Debug` object
  - string constants for MQTT JSON keys
- `src/mqtt_com.h`:
  - Owns the `EspMQTTClient client`.
  - `onConnectionEstablished()` subscribes to `LIGHT_SET_TOPIC` and `LIGHT_SET_TOPIC_GROUP`, and publishes HA discovery when that flag is on.
  - `callBack()` parses JSON commands (`state` ON/OFF, `effect`, `brightness`, `color{r,g,b}`, `speed`). It changes `desiredState` / `cluster`, then `sendState()` publishes to `LIGHT_STATE_TOPIC`.
  - `speed` (1..255, 128 = normal) sets the animation rate and is reported in state. HA's `transition` (seconds) is ignored; HA's UI can't set `speed`, so send it from an automation with `mqtt.publish`.
- `setup()` blocks until MQTT connects, then starts ArduinoOTA and RemoteDebug. That's why telnet answering means MQTT is up.
  - Anything that must happen before the first `client.loop()` goes above the connect loop: `setMaxPacketSize`, `enableLastWillMessage`, the initial `cluster.setMode`.
  - DEBUG output from inside the connect loop (including `onConnectionEstablished` on first connect) never reaches telnet.
- The device boots with power OFF until it gets an ON command.
- Power cap: `FastLED.setMaxPowerInVoltsAndMilliamps(5, PSU_MA)`. The supply is 5V/15A; `PSU_MA` defaults to 12000 in `globals.h`.
- `loop()`: OTA handle → `client.loop()` → if powered on, `cluster.loop()`. If that returns true (an effect such as `alert` ended itself), `sendState()`.
- Effects live in `HexlightCluster.cpp`:
  - **Adding an effect** = write an `fx*(const Frame &)` function and add one `{name, fn}` row to `EFFECTS[]`. MQTT matching, state reporting and the HA `effect_list` all read that table.
  - `cluster.loop()` renders one frame every 20ms. The effect draws into each panel's `hexLights[]` canvas, then every panel `syncAll()`s and `FastLED.show()` runs once.
  - `Frame::t` is effect time in ms, scaled by `speed` and reset to 0 by `setMode`. `Frame::first` marks the first frame, for resetting static state. `Ticker` gives periodic steps on that clock.
  - Returning true from an fx ends it and goes back to the previous mode (used by `alert`).
- `src/geometry.h` (no FastLED, host-testable):
  - `PANEL_XY` holds each panel's {column, row} from the layout map, which is a comment at the top of the file.
  - `initGeometry()` fills per-LED wall coordinates `GX/GY` (0..255, y up), distance `GD` and angle `GA` about the cluster centre, plus panel centres `PCX/PCY/PD` and a `NEIGHBORS` bitmask.
  - Calibration knobs: `LED_ZERO_DEG` and `LED_DIR`. The code assumes logical LED 0 is bottom centre, running counterclockwise; change these if 2D effects come out mirrored or rotated relative to `horizontal`.
- `Hexlight` (`hexlight.h/.cpp`) is one 35-LED panel.
  - `hexLights[]` is in logical order; the constructor's orientation (sides rotated) maps it to physical LEDs.
  - `setLight(angle)` takes any int angle in degrees from LED 0.
  - `waveLightStep(pos)` draws a horizontal line at height 0..15.
  - The panel table `HEX_LIGHTS` (start index, orientation) is in `HexlightCluster.cpp`.
- MQTT broker: 192.168.1.115. Topics: `home/houselet_light1` (state), `home/houselet_light1/set` (commands), `home/houselet_light1/availability`, `homeassistant/device/houselet_light1/config`. To test, use `mosquitto_pub` / `mosquitto_sub` with the light1 credentials from `secrets.json`.

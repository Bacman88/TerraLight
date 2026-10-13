# TerraLight firmware v2

ESP32-C3 SuperMini · PlatformIO · ESP32 Arduino core 3.3 · MIT

Design: [../docs/ARCHITECTURE.md](../docs/ARCHITECTURE.md) · Features: [../docs/FIRMWARE.md](../docs/FIRMWARE.md)

## Status — M1 (skeleton) · `2.0.0-dev.1`

| Works | Not yet |
|---|---|
| Settings in NVS (schema-versioned, deferred save) | Schedule, sun, moon, scenes (M2) |
| Clock: NTP + POSIX time zone (DST correct), time from the phone | Sensors (M3) |
| Wi-Fi: non-blocking, retries without falling back to AP | Biotopes, climate, Aqua scenes (M4) |
| Setup AP + captive portal — light keeps running | Observation / service / demo, OTA, backups (M5) |
| BOOT button held 5 s → forget Wi-Fi, start AP | |
| LED output: module presets, colour order, density, gamma, current limiter, soft start | |
| Static light (on/off, colour, brightness, density) | |
| Web panel PL/EN + REST API | |

## Build and flash (PlatformIO)

1. Install **VS Code** or **VSCodium** and the **PlatformIO IDE** extension.
2. *File → Open Folder…* → the `firmware/` folder of this repository.
   The first open downloads the ESP32 platform (a few minutes, once).
3. Connect the SuperMini with a USB-C cable.
4. Click **→ Upload** in the bottom status bar (✓ Build only compiles).
5. **Serial Monitor** (🔌 in the status bar) shows the log at 115200 baud.

If the board sees Wi-Fi networks but never connects, some SuperMini batches need lower TX power:
uncomment `-DTL_WIFI_TX_POWER=WIFI_POWER_8_5dBm` in `platformio.ini`.

If the board does not show up as a port: hold **BOOT**, tap **RESET**, release BOOT, upload again.

The web panel (`web/`) is packed into `src/web/assets.h` automatically before every build
(`tools/build_assets.py`). The generated header is committed, so the project also builds without Python.

## First start

1. The light fades in over ~60 s (soft start) in warm white.
2. No saved network → Wi-Fi **`TerraLight-Setup`**, password **`terralight`**.
   Phones usually open the setup page by themselves; otherwise go to **http://192.168.4.1**.
3. Pick your 2.4 GHz network, enter the password, *Connect*. The setup network closes ~30 s after a successful connection.
4. Panel: **http://terralight.local** (or the IP address shown on the setup page / in your router).

Onboard blue LED: slow blink = setup mode · fast blink = connecting · steady while BOOT is held · off = connected.

## M1 test checklist

- [ ] Light fades in after power-up; colour, brightness and density change live from the panel.
- [ ] Wrong colours → change *Colour order* (default RGB) — note which order the module needs.
- [ ] Setup AP: the light keeps running while the setup page is open.
- [ ] Wrong password → error on the setup page, AP stays up; correct password → connects, AP closes.
- [ ] Router off for a few minutes → TerraLight does **not** open the setup AP, reconnects when the router is back.
- [ ] BOOT held 5 s → blue LED steady from ~1 s, then setup mode (slow blink).
- [ ] Time correct in the panel (Europe/Warsaw, summer/winter time).
- [ ] *Set time from this phone* works without internet (AP mode).
- [ ] LED budget: full white on a LINE 30 module → panel shows "limited to the supply budget".
- [ ] Settings survive a power cycle.

## REST API (M1)

| Method | Path | |
|---|---|---|
| GET | `/api/status` | version, time, Wi-Fi, LED current estimate |
| GET / POST | `/api/config` | settings; POST accepts partial JSON, e.g. `{"light":{"brightness":40}}` |
| POST | `/api/time` | `{"epoch": 1760000000}` |
| GET | `/api/wifi/scan` | `?refresh=1` starts a new scan |
| POST | `/api/wifi` | `{"ssid":"…","password":"…"}` |
| POST | `/api/wifi/forget` | forget network, start setup AP |
| POST | `/api/system/restart` | |
| POST | `/api/system/defaults` | `{"profile":"terra"\|"aqua"}` — Wi-Fi is kept |

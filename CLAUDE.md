# TerraLight — notes for Claude

Open-source (MIT) LED day/night & climate controller for terrariums, aquariums, paludariums and formicariums.
Owner: Bartosz / Smart Ant Technologies (GitHub `Bacman88`). Talk to the owner in **Polish**; code, comments and commit messages in **English**.

## Where things are
- `docs/FIRMWARE.md` — features and decisions (source of truth for *what* to build; Polish)
- `docs/ARCHITECTURE.md` — *how* it is built: modules, render pipeline, REST API, milestones M1–M5
- `docs/HARDWARE.md`, `docs/SENSOR_MINI.md`, `docs/LED_MODULES.md` — hardware (ESP32-C3 SuperMini, pinout candidates)
- `firmware/` — PlatformIO project (ESP32 Arduino core 3.3.12 via pioarduino `55.03.312-1`)
- `firmware/web/` — panel sources; packed into `firmware/src/web/assets.h` by `firmware/tools/build_assets.py` (pre-build script)

## Build / flash (Windows, PlatformIO IDE)
`pio` is usually NOT on PATH. Use the PlatformIO IDE buttons, or the full path:
```
%USERPROFILE%\.platformio\penv\Scripts\pio.exe run -d firmware                 # build
%USERPROFILE%\.platformio\penv\Scripts\pio.exe run -d firmware -t upload       # flash over USB
%USERPROFILE%\.platformio\penv\Scripts\pio.exe device monitor -d firmware      # serial log, 115200
```
Board not visible as a port → hold BOOT, tap RESET, release BOOT, upload again.

## Git workflow (several computers: work / home)
GitHub `Bacman88/TerraLight` is the sync point between machines; author identity is `Bacman <bacmanowski@gmail.com>`.
- At session start: `git fetch` + `git status`; if behind `origin/main`, pull before editing anything.
- At the end of a work session: offer to commit and push, so the other machine gets the changes.
- New machine: `git clone https://github.com/Bacman88/TerraLight.git` (never a ZIP copy — it has no `.git`).
- Line endings are LF (`.gitattributes`); don't convert files to CRLF.

## Rules for the firmware
- ESP32-C3 is single-core: **nothing in `loop()` may block** (no long `delay()`, no `while` waiting for the network). Network fetches go to a separate FreeRTOS task.
- The light must keep running in every Wi-Fi state, including the setup AP.
- Scenes render perceptual colour into `FrameBuffer`; density, brightness limits, gamma, current limiter and colour order belong to `light/led_output` only.
- Settings struct changes → bump `SETTINGS_SCHEMA` and add a migration in `core/settings.cpp`; OTA must never wipe user settings.
- Keep REST/JSON field names stable (panel and backups depend on them).
- Panel must work offline (AP mode): no CDN for core UI.
- After changing `firmware/web/*`, the build regenerates `assets.h` — commit both.
- Build with no errors and no new warnings before committing.

## Status
- M1 skeleton done (`2.0.0-dev.1`): settings, clock with POSIX TZ, non-blocking Wi-Fi + captive portal, BOOT reset, LED output, static light, panel + API.
- Next: hardware test of M1 (checklist in `firmware/README.md`), then M2 — schedule, sun/moon calculation, day scenes, night, demo.

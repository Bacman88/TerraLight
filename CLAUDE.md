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

Windows long paths: the ESP32 Arduino libs contain paths over 260 chars, so installing the platform
fails with `FileNotFoundError` under `%USERPROFILE%\.platformio` unless long paths are enabled.
Fix without admin: set the user env var `PLATFORMIO_CORE_DIR=C:\pio` (no trailing space) and restart
VS Code; packages then go to `C:\pio` (the `penv` with `pio.exe` may stay in `%USERPROFILE%\.platformio`).

## Release (OTA)
1. Bump `TL_FW_VERSION` in `firmware/src/core/version.h`, build, commit and push.
2. `python firmware/tools/make_release.py` → `firmware/release/v<version>/{firmware.bin,manifest.json}` (git-ignored).
3. `gh release create v<version> firmware/release/v<version>/firmware.bin firmware/release/v<version>/manifest.json --target main` — NOT `--prerelease` (devices read `releases/latest`).
Releases are public and devices see them at once — ask the owner before publishing. `gh` is installed on the work PC
(`C:\Program Files\GitHub CLI\gh.exe`, logged in as Bacman88); on another machine install and `gh auth login` first.

## Git workflow (several computers: work / home)
GitHub `Bacman88/TerraLight` is the sync point between machines; author identity is `Bacman <bacmanowski@gmail.com>`.
- At session start: `git fetch` + `git status`; if behind `origin/main`, pull before editing anything.
- At the end of a work session: update the **Status** section below (done / next / half-finished), record important decisions in the right `docs/*.md`, then offer to commit and push, so the other machine gets the changes. Claude's chat history and local memory do not travel between machines — only the repo does.
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
- 2026-10-14: M1 flashed and running on an ESP32-C3 SuperMini (COM6). Wi-Fi setup works (fixed an unsigned-timer bug that failed every test instantly), `terralight.local` works, full TX power is fine (RSSI −47).
- 2026-10-14 also: Wi-Fi diagnostics (router disconnect reason in log + panel), show-password toggle, SSID from list fills the text field; first-run wizard in AP mode only (Wi-Fi/skip → LED count + supply), `SETTINGS_SCHEMA` 2 with migration; panel restyled like WeatherLight (dark navy + cyan).
- 2026-10-14 later: OTA from GitHub Releases without a token (`net/ota`, `2.0.0-dev.2`, flashed). First release `v2.0.0-dev.3` published with `gh`; the board found it on its own, downloaded ~1.3 MB in about a minute with the light running, verified SHA-256, rebooted into dev.3 and confirmed the image after 60 s. OTA works end to end.
- Note: opening COM6 from a script resets this board (RTS), so restarting a serial listener reboots it.
- `2.0.0-dev.4`: hard LED current limits (`LED_OUT_MAX_MA` 2.5 A, min 10 mA/px), limiter hole fixed, wizard offers supplies up to 3 A only. Decided: power via USB-C (no PD) instead of a DC jack — see HARDWARE.md "Granice". Customer compatibility doc: `docs/KOMPATYBILNOSC.md` (draft).
- Serial boot log: the board prints only at start; to capture it, listen on COM6 and press RESET (RTS reset from the PC does not work on this board).
- 2026-10-15: PCB design proposal `docs/PCB.md` (v0.2) — production board with an ESP32-C3-MINI-1 module soldered on (not SuperMini), one USB-C 16-pin for power + programming, CC/VBUS sensing on ADC, LED power switch on GPIO20, JLCPCB SMD assembly. Open decisions listed there; schematic to be drawn in KiCad 9 (owner may continue in Claude on the web). Concept additions: main lamp (UVB) cooperation option, Sensor PRO outputs (heating, UVB/night lamp).
- 2026-10-15: KiCad 9 schematic of the main board generated from `docs/PCB.md` → `schematy/kicad9/TerraLight-main/` (connectivity verified by script; still to do in KiCad: ERC, then PCB layout). Prototypes are hand-assembled by the owner (beginner): module switched to **ESP32-C3-WROOM-02-N4** (castellated pads, GND pad soldered through a plated hole), passives 0805, MPN fields + `BOM_prototyp_5szt.csv`. GPIO assignment unchanged.
- Next: test the wizard; add the logo (owner will provide SVG/PNG, ~860 kB flash free); remaining M1 hardware checklist (`firmware/README.md`); then M2 — schedule, sun/moon, day scenes, night, demo.

#pragma once
// ─────────────────────────────────────────────────────────────
//  TerraLight — pinout ESP32-C3 SuperMini
//  Source: docs/HARDWARE.md (candidates — confirm against the board
//  schematic before making the PCB).
//
//  Do NOT use: GPIO2, GPIO8, GPIO9 (boot strapping pins — GPIO8 and
//  GPIO9 only for the purposes below), GPIO18/19 (USB), GPIO20/21 (UART).
// ─────────────────────────────────────────────────────────────
#include <Arduino.h>

namespace pins {
constexpr uint8_t LED_DATA = 4;    // → 74HCT125 → 330 Ω → DIN of the LED module
constexpr uint8_t DATA1 = 5;       // SENSOR pin 3 — DHT22 A / DS18B20
constexpr uint8_t DATA2 = 6;       // SENSOR pin 4 — DHT22 B / DS18B20
constexpr uint8_t AUX1 = 7;        // SENSOR pin 5 — reserved / I²C SDA
constexpr uint8_t AUX2 = 10;       // SENSOR pin 6 — reserved / I²C SCL
constexpr uint8_t BUTTON = 9;      // BOOT button of the SuperMini (active LOW)
constexpr uint8_t STATUS_LED = 8;  // onboard blue LED of the SuperMini (active LOW)
}  // namespace pins

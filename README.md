# TerraLight

Modułowy system do terrariów, formikariów i akwariów (tryby **Terra** i **Aqua**): symulacja dnia i nocy za pomocą adresowalnych LED oraz opcjonalny **Sensor MINI** z dwiema przewodowymi sondami temperatury i wilgotności.

**Status:** dokumentacja sprzętowa v0.1, etap projektowania prototypu. Schemat PCB i firmware nie są jeszcze gotowe.

## Pierwsza wersja
- Sterownik: ESP32-C3 SuperMini, Wi-Fi 2,4 GHz, zasilanie **5 V / 2 A**.
- LED: wymienne zestawy WS2812B / WS2811 (RING 8, RING 12, ARC 15–20 i LINE 60).
- SENSOR: 6-pinowe złącze rozbudowy 3,3 V.
- Sensor MINI: dwa niezależne kanały sond, każdy DHT22/AM2302 lub DS18B20; można mieszać typy (np. powietrze i temperatura powierzchni).
- Tryb Aqua: wodoodporna sonda temperatury DS18B20, również na kanale DATA1/DATA2.
- Bez przekaźników i bez napięcia sieciowego na podstawowej płytce.
- Bufor 74HCT125 na linii DATA LED (3,3 V → 5 V) obowiązkowy w prototypie; wymagane testy przed produkcją.

## Dokumentacja
- [Hardware — architektura i budżet zasilania](docs/HARDWARE.md)
- [Sensor MINI — pinout 6-pin i sondy DHT22](docs/SENSOR_MINI.md)
- [Wymienne moduły LED](docs/LED_MODULES.md)
- [Firmware v2.0 — ustalenia i znane błędy](docs/FIRMWARE.md)
- [Roadmap — Sensor PRO, ESP-NOW, Hub](docs/ROADMAP.md)

## Firmware
Firmware rozwijany osobno przez właściciela projektu z Claude'em, w oparciu o [WeatherLight](https://github.com/Bacman88/WeatherLight). Zakładane funkcje: świt i zmierzch, cykl dnia, fazy księżyca i chmury, konfiguracja liczby i typu LED, dane pogodowe Open-Meteo (opcjonalnie), dwa pomiary temperatury i wilgotności oraz panel Wi-Fi.

## Przyszłość
Osobny **Sensor PRO** z własnym sterownikiem i wyjściami automatyki, połączony z TerraLight przez ESP-NOW (z zachowaniem zgodności kanałów z domowym Wi-Fi). Hub pozostaje opcjonalnym pomysłem.

Copyright (c) 2026 Smart Ant Technologies. Zobacz [LICENSE](LICENSE).

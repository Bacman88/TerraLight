# TerraLight

**Open-source ESP32-C3 LED day/night & climate simulator for terrariums, aquariums, paludariums and formicariums.**
by [Smart Ant Technologies](https://smart-ant.pl)

[English](#english) · [Polski](#polski)

---

## English

TerraLight is a modular lighting controller for vivariums. Addressable LEDs simulate sunrise, daylight, sunset and night — including a moving sun, real moon phases and a programmed (or live) climate with clouds, rain and storms. Two device profiles: **Terra** (terrariums, paludariums, ant formicariums) and **Aqua** (aquariums).

**Status:** hardware documentation v0.1, prototype design stage. PCB and firmware are in progress.

### Highlights
- **Controller:** ESP32-C3 SuperMini, 2.4 GHz Wi-Fi, single **5 V / 2 A** supply.
- **LED modules:** swappable WS2812B / WS2811 sets — RING 8, RING 12, ARC 15–20, LINE.
- **Light modes:** static or dynamic, manual or scheduled; full-module sunrise → day → sunset, moving sun with configurable size, start point and path, plant light, cloudy jungle, rain and optional storm.
- **Schedules:** fixed hours or real sunrise/sunset calculated locally for any **biotope location** (e.g. Australia for a bearded dragon), seasonal day shortening / brumation, soft start after power loss.
- **Night:** moonlight with real moon phases, or red **observation light** that does not disturb ants and nocturnal animals.
- **Aqua mode:** water caustics, midday siesta, blackwater / Malawi / reef biotopes, service light for feeding and cleaning.
- **Climate:** biotope presets with randomized programmed weather, or optional live weather (Open-Meteo) for the biotope or your home location — always within animal-welfare limits.
- **Sensors:** two independent channels, each **DHT22** (air temperature & humidity) or **DS18B20** (water or basking-spot temperature); mix and match, e.g. air + water in a paludarium.
- **Works offline:** time, sun and moon are calculated on the device; Wi-Fi only for clock sync, OTA updates and optional live weather.
- No relays and no mains voltage on the base board. 74HCT125 level shifter on the LED data line.

### Documentation
Documentation is currently in Polish.
- [Hardware — architecture and power budget](docs/HARDWARE.md)
- [Sensor MINI — 6-pin port, DHT22 / DS18B20](docs/SENSOR_MINI.md)
- [LED modules](docs/LED_MODULES.md)
- [Firmware v2.0 — features and decisions](docs/FIRMWARE.md)
- [Firmware architecture](docs/ARCHITECTURE.md)
- [Roadmap — Sensor PRO, ESP-NOW, Hub](docs/ROADMAP.md)

### Firmware
Fully **open source**. The firmware (PlatformIO, ESP32 Arduino core) is developed in the `firmware/` folder of this repository, based on [WeatherLight](https://github.com/Bacman88/WeatherLight). Ready-to-flash binaries will be published in [Releases](../../releases).

### Future
A separate **Sensor PRO** with its own controller and automation outputs (heating, ventilation, misting, lighting), linked to TerraLight via ESP-NOW. An optional Hub for managing multiple units.

> RGB LEDs do not replace species-appropriate daylight, UVB lighting or grow lights for plants and corals.

---

## Polski

TerraLight to modułowy sterownik oświetlenia dla terrariów, formikariów, paludariów i akwariów. Adresowalne LED symulują świt, dzień, zachód i noc — z wędrującym słońcem, rzeczywistymi fazami księżyca oraz zaprogramowanym (lub rzeczywistym) klimatem: chmurami, deszczem i burzą. Dwa profile urządzenia: **Terra** (terraria, paludaria, formikaria) i **Aqua** (akwaria).

**Status:** dokumentacja sprzętowa v0.1, etap projektowania prototypu. Schemat PCB i firmware w przygotowaniu.

### Pierwsza wersja
- Sterownik: ESP32-C3 SuperMini, Wi-Fi 2,4 GHz, zasilanie **5 V / 2 A**.
- LED: wymienne zestawy WS2812B / WS2811 (RING 8, RING 12, ARC 15–20 i LINE 60).
- SENSOR: 6-pinowe złącze rozbudowy 3,3 V.
- Sensor MINI: dwa niezależne kanały sond, każdy DHT22/AM2302 lub DS18B20; można mieszać typy (np. powietrze i temperatura powierzchni).
- Tryb Aqua: wodoodporna sonda temperatury DS18B20, również na kanale DATA1/DATA2.
- Bez przekaźników i bez napięcia sieciowego na podstawowej płytce.
- Bufor 74HCT125 na linii DATA LED (3,3 V → 5 V) obowiązkowy w prototypie; wymagane testy przed produkcją.

### Najważniejsze funkcje
- Świecenie stałe lub dynamiczne, ręcznie lub wg harmonogramu; dzień na całym module, wędrujące słońce, światło dla roślin, pochmurna dżungla, deszcz i opcjonalna burza.
- Harmonogram: stałe godziny lub rzeczywisty wschód i zachód liczony lokalnie dla **lokalizacji biotopu**, sezonowość / zimowanie, łagodny start po zaniku zasilania.
- Noc: poświata z rzeczywistymi fazami księżyca albo czerwone **światło obserwacyjne**, które nie stresuje mrówek.
- Tryb Aqua: refleksy wody, sjesta, biotopy (Amazonka, Malawi, rafa), tryb serwisowy.
- Działa bez internetu; Wi-Fi tylko do synchronizacji czasu, aktualizacji OTA i opcjonalnej pogody na żywo.

### Dokumentacja
- [Hardware — architektura i budżet zasilania](docs/HARDWARE.md)
- [Sensor MINI — pinout 6-pin i sondy](docs/SENSOR_MINI.md)
- [Wymienne moduły LED](docs/LED_MODULES.md)
- [Firmware v2.0 — ustalenia i znane błędy](docs/FIRMWARE.md)
- [Architektura firmware](docs/ARCHITECTURE.md)
- [Roadmap — Sensor PRO, ESP-NOW, Hub](docs/ROADMAP.md)

### Firmware
Projekt w pełni **open source** — kod firmware (PlatformIO, ESP32 Arduino) powstaje w katalogu `firmware/` tego repo, na bazie [WeatherLight](https://github.com/Bacman88/WeatherLight). Gotowe pliki do wgrania będą publikowane w [Releases](../../releases).

### Przyszłość
Osobny **Sensor PRO** z własnym sterownikiem i wyjściami automatyki, połączony z TerraLight przez ESP-NOW (z zachowaniem zgodności kanałów z domowym Wi-Fi). Hub pozostaje opcjonalnym pomysłem.

> RGB nie zastępuje wymaganego gatunkowo światła dziennego, UVB ani lamp do uprawy roślin i koralowców.

---

Copyright (c) 2026 [Smart Ant Technologies](https://smart-ant.pl). MIT License — see [LICENSE](LICENSE).

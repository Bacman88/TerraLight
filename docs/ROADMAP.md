# TerraLight — plan rozwoju

## v0.1 Hardware — teraz
- ESP32-C3 SuperMini, zasilacz 5 V / 2 A.
- LED OUT 3-pin, SENSOR 6-pin, 2 × DHT22 na przewodach.
- Brak przekaźnika na PCB podstawowym.
- Konwerter poziomów DATA LED **obowiązkowo**: na etapie prototypu przewlekany 74HCT125 DIP-14 (alternatywnie sprawdzony gotowy moduł 74AHCT125/74HCT125).
- Schemat KiCad i PCB dopiero po zweryfikowaniu pinoutu konkretnej płytki, złączy, prądu i próbek.

## Sonda DS18B20 — woda (Aqua) i miejsce wygrzewania (Terra)
- Wodoodporna sonda temperatury DS18B20 (1-Wire) dla trybu Aqua, podłączana **w miejsce DHT22** na DATA1 / DATA2 portu SENSOR (to samo złącze i ten sam rezystor podciągający 4,7 kΩ do 3,3 V). AUX1/AUX2 pozostają wolne dla I²C (np. RTC).
- Firmware: typ czujnika na każdym kanale wybierany w ustawieniach (DHT22 / DS18B20 / brak). Na jednej linii 1-Wire może pracować kilka sond DS18B20 (unikalne adresy).
- Do weryfikacji: długość przewodu, jakość uszczelnienia sondy, mocowanie do kamienia / w podłożu, dokładność partii (na rynku dużo podróbek DS18B20 — testować próbki od dostawcy).

## Firmware — użytkownik + Claude
Kod będzie bazował na wcześniejszym WeatherLight: https://github.com/Bacman88/WeatherLight. Konfiguracja typu i liczby LED, efekty i harmonogramy, odczyt 2 × DHT22, obsługa błędów i ograniczenia poboru, lokalny panel Wi-Fi. Nie dodajemy firmware automatycznie w tym etapie.

## Sensor PRO — przyszłość
Własny ESP32 i zasilanie, czujniki temperatury/wilgotności, sterowanie zewnętrznymi elementami wykonawczymi (ogrzewanie, wentylacja, zraszanie, światło); bezpieczne stany autonomiczne. ESP-NOW do łączności z TerraLight; jednoczesne Wi-Fi+ESP-NOW przy tym samym kanale, wymagane ponowne odnajdywanie przy zmianach kanału routera.

Minimum: **dwa wyjścia** (przekaźnik / SSR, 230 V tylko na module PRO, nigdy na PCB TerraLight):
- **Grzanie** — termostat wg czujnika (DHT22 / DS18B20 pod lampą), osobne progi dzień / noc, twardy limit za gorąco.
- **Lampa UVB / nocna** — dla gadów, które tego wymagają: w dzień UVB włączane i wyłączane przez harmonogram TerraLight (koniec świtu → początek zmierzchu, patrz FIRMWARE.md „Współpraca z lampą główną”), albo w nocy lampa nocna / grzewcza bez światła widzialnego.

Przy utracie łączności z TerraLight moduł pracuje na ostatnim znanym harmonogramie z własnego zegara; grzanie zawsze ograniczone własnym czujnikiem.

## Hub — opcjonalnie
Zarządzanie wieloma TerraLight i Sensor PRO. Nie stanowi zależności dla podstawowej funkcjonalności.

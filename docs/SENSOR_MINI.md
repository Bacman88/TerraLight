# TerraLight Sensor MINI v0.1

MINI to **dwie gotowe przewodowe sondy DHT22/AM2302**, dla cieplejszej i chłodniejszej strefy terrarium. Bez własnego ESP32, zasilacza, Wi-Fi i przekaźników.

## Podłączenie
Jeden przewód ze złączem SENSOR 6-pin wychodzi z TerraLight i rozdziela się na dwie 3-żyłowe wiązki.
- Pin 1 3V3 → oba czujniki VCC.
- Pin 2 GND → oba GND.
- Pin 3 DATA1 → czujnik A.
- Pin 4 DATA2 → czujnik B.
- Piny 5 AUX1 i 6 AUX2 wolne dla przyszłych modułów (np. I²C).

DHT22 **nie jest I²C**: każdy ma osobny sygnał DATA, więc dwa takie same czujniki nie mają konfliktu adresów.

## Rezystory pull-up
Na DATA1 i DATA2 przewidzieć osobne podciąganie do 3,3 V, prototypowo 4,7 kΩ; footprinty konfigurowalne, bo gotowe moduły mogą już mieć własne rezystory. Weryfikować przy długościach i typach przewodów używanych przez klienta.

## Głowice i środowisko
Głowice wentylowane na przewodach, umieszczane niezależnie; nie zalewać żywicą części pomiarowej. Unikać bezpośredniego zraszania i kondensacji. DHT22 nie jest sondą zanurzeniową ani czujnikiem temperatury powierzchni miejsca wygrzewania — do tego służy sonda DS18B20 podłączana w miejsce jednego z DHT22 (patrz FIRMWARE.md, „Czujniki”). Firmware musi oznaczać awarie, brak pomiaru i błędne sumy kontrolne; nie używać starych danych jako aktualnych.

## Rozbudowa
Na AUX1/AUX2 możliwe późniejsze SDA/SCL I²C, pod warunkiem doboru czujników z różnymi adresami i odpowiedniego podciągania. Sensor PRO pozostaje niezależnym urządzeniem z ESP32 i komunikacją ESP-NOW.

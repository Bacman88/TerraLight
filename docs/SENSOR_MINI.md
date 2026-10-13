# TerraLight Sensor MINI v0.1

MINI udostępnia **dwa niezależne kanały sond**: każdy może obsługiwać DHT22/AM2302 (temperatura i wilgotność), DS18B20 (temperatura wody lub powierzchni wygrzewania) albo pozostawać pusty. Typ wybierany osobno w firmware. Bez własnego ESP32, zasilacza, Wi-Fi i przekaźników.

## Podłączenie
Jeden przewód ze złączem SENSOR 6-pin wychodzi z TerraLight i rozdziela się na dwie 3-żyłowe wiązki.
- Pin 1 3V3 → oba czujniki VCC.
- Pin 2 GND → oba GND.
- Pin 3 DATA1 → czujnik A (DHT22 lub DS18B20).
- Pin 4 DATA2 → czujnik B (DHT22 lub DS18B20).
- Piny 5 AUX1 i 6 AUX2 wolne dla przyszłych modułów (np. I²C).

DHT22 **nie jest I²C**. DS18B20 pracuje w 1-Wire; przy zgodnym wariancie zasilania może korzystać z tej samej linii DATA co DHT22, ale **nie w tym samym momencie / na tej samej magistrali**. Na jednej linii DS18B20 można obsłużyć wiele urządzeń z unikalnymi identyfikatorami.

## Rezystory pull-up
Na DATA1 i DATA2 przewidzieć osobne podciąganie do 3,3 V, prototypowo 4,7 kΩ. Pasuje jako punkt startowy zarówno dla DHT22, jak i DS18B20 z zasilaniem 3-przewodowym. Footprinty konfigurowalne, bo gotowe moduły mogą już mieć własne rezystory. Weryfikować przy długościach i typach przewodów używanych przez klienta.

## Głowice i środowisko
Głowice wentylowane na przewodach, umieszczane niezależnie; nie zalewać żywicą części pomiarowej. Unikać bezpośredniego zraszania i kondensacji. DHT22 nie jest sondą zanurzeniową ani czujnikiem temperatury powierzchni miejsca wygrzewania — do tego służy sonda DS18B20 podłączana w miejsce jednego z DHT22 (patrz FIRMWARE.md, „Czujniki”). Firmware musi oznaczać awarie, brak pomiaru i błędne sumy kontrolne; nie używać starych danych jako aktualnych.

## Rozbudowa
Na AUX1/AUX2 możliwe późniejsze SDA/SCL I²C, pod warunkiem doboru czujników z różnymi adresami i odpowiedniego podciągania. Sensor PRO pozostaje niezależnym urządzeniem z ESP32 i komunikacją ESP-NOW.

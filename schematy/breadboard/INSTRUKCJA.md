# TerraLight — minimum na breadboardzie (ESP32-C3 SuperMini)

## Minimalny BOM
| Ilość | Element |
|---|---|
| 1 | ESP32-C3 SuperMini z USB-C |
| 1 | 74HCT125 DIP-14 (Pixel Boost) + opcjonalnie podstawka |
| 1 | rezystor 330 Ω |
| 1 | kondensator ceramiczny 100 nF |
| 1 | kondensator elektrolityczny 470–1000 µF, co najmniej 10 V |
| 1 | moduł 6–12 adresowalnych LED WS2812B/WS2811 **5 V** |
| — | przewody, breadboard, zasilanie USB 5 V |
| opcj. | 2 × DHT22 lub DS18B20 + rezystory 4,7 kΩ |

## Połączenia Pixel Boost — 74HCT125 DIP-14
Patrząc z góry na scalak z wycięciem/znacznikiem u góry, piny liczone przeciwnie do ruchu wskazówek zegara:
- **14 VCC → +5 V**
- **7 GND → GND**
- **1 (1/OE, aktywne LOW) → GND**
- **2 (1A) ← GPIO4 ESP32**
- **3 (1Y) → rezystor 330 Ω → DIN LED**
- **100 nF między pin 14 i pin 7**, blisko układu.
- Wyłącz niewykorzystane bufory: **4, 10, 13 (/OE) → +5 V**; **5, 9, 12 (A) → GND**; wyjścia **6, 8, 11 (Y) niepodłączone**.

**Nie używaj 74HC125 zamiast 74HCT125**, jeśli bufor zasilany jest 5 V — HCT rozpoznaje poziom wysoki 3,3 V z ESP32.

## Zasilanie LED
- LED **+5 V** → zasilanie USB 5 V/VBUS (po sprawdzeniu obciążalności SuperMini).
- LED **GND** → GND wspólne z ESP32 i buforem.
- 470–1000 µF **równolegle między +5 V i GND przy LED** (elektrolit: plus do +5 V).
- **Nie podłączaj +5 V LED do GPIO ani do 3V3.**
- Przy sześciu WS2812B pełna biel może pobrać ~360 mA; firmware ogranicz na początek do 20–30%. Nie zakładaj, że ścieżki/złącze każdego klonu SuperMini wytrzymają pełny prąd.
- Nie łącz niezależnego zasilacza 5 V z USB równolegle bez rozważenia zabezpieczenia przed zasilaniem zwrotnym.

## Opcjonalne sondy
- DHT22 A: VCC 3V3; GND; DATA GPIO5 z pull-up 4,7 kΩ do 3V3.
- DHT22 B: VCC 3V3; GND; DATA GPIO6 z pull-up 4,7 kΩ do 3V3.
- Na obu kanałach może być zamiast DHT22 3-przewodowy DS18B20 przy prawidłowej konfiguracji firmware (także z 4,7 kΩ).
- Napięcie 5 V **nie może trafić na GPIO ESP32**.

## Uwagi
To instrukcja logicznych połączeń. Fizyczna numeracja wyprowadzeń ESP32-C3 SuperMini zależy od wersji płytki. Przed uruchomieniem sprawdzić nadruk GPIO4, 5V, GND oraz kierunek DIN/DO LED. Breadboard nie służy do prowadzenia kilku amperów.

# TerraLight — hardware v0.1

Status: projekt koncepcyjny do prototypowania, nie gotowa PCB.

## Architektura
- ESP32-C3 SuperMini, Wi-Fi 2,4 GHz; przyszły ESP-NOW.
- Jedno zasilanie 5 V przez gniazdo USB-C (ładowarka 2–3 A, bez PD — patrz „Granice”). Dwa równoległe obwody: zasilanie ESP przez wejście 5 V oraz zasilanie LED przez wyjście LED OUT. Wspólna masa.
- LED OUT: +5V / DATA / GND, jeden wymienny moduł LED na raz.
- SENSOR: port sześciopinowy 3,3 V z dwiema liniami DHT i dwiema liniami rezerwowymi.
- Brak przekaźników i obwodów 230 V w TerraLight v1.
- USB-C ESP dostępne do programowania.
- Przycisk RESET Wi-Fi: wbudowany przycisk BOOT płytki SuperMini (GPIO9) dostępny przez otwór w obudowie (szpilka). Bez dodatkowego elementu na PCB.
- Projekt przewiduje rezystor DATA ok. 330 Ω, kondensator na LED 470–1000 µF / min. 10 V, ceramikę odsprzęgającą 100 nF i obowiązkowy konwerter poziomów LED 3,3 V → 5 V — dla prototypu przewlekany 74HCT125 (DIP-14), ewentualnie sprawdzony gotowy moduł z buforem 74AHCT125/74HCT125.
- Dobrać zabezpieczenie zasilania (odwrotna polaryzacja, nadprąd, ESD) i szerokości ścieżek po ustaleniu PCB.

## Bufor LED — wymagany w prototypie
Przewlekany **74HCT125 DIP-14**, zasilany 5 V (nie mylić z 74HC125; rodzina HCT akceptuje poziom wysoki 3,3 V na wejściu przy zasilaniu 5 V). Używamy tylko jednego z czterech buforów:
- pin 14 VCC → +5 V; pin 7 GND → masa wspólna; 100 nF bezpośrednio między 14 i 7;
- pin 1 (1/OE, aktywny stanem niskim) → GND; pin 2 (1A) → GPIO4 ESP32-C3; pin 3 (1Y) → rezystor szeregowy 330 Ω → LED DATA/DIN;
- nieużywane bufory wyłączyć: wejścia 2/OE (pin 4), 3/OE (pin 10), 4/OE (pin 13) → +5 V; ich wejścia A (piny 5, 9, 12) → GND; wyjścia Y (piny 6, 8, 11) pozostawić niepodłączone.
- Wersja z gotowym modułem konwertera: stosować **bufor AHCT/HCT do LED**, nie typowy dwukierunkowy BSS138 przeznaczony do I²C. Sprawdzić pinout konkretnego modułu.
Bufor chroni zgodność logiczną i stabilność DATA, **nie jest zabezpieczeniem przed nadnapięciem zasilania 5 V ani odwrotną polaryzacją**.

## Port SENSOR v1
| Pin | Nazwa | Rola |
|---|---|---|
| 1 | 3V3 | niskoprądowe zasilanie sond |
| 2 | GND | wspólna masa |
| 3 | DATA1 | sonda DHT22 A **lub** DS18B20 (1-Wire, Aqua) |
| 4 | DATA2 | sonda DHT22 B **lub** DS18B20 (1-Wire, Aqua) |
| 5 | AUX1 | rezerwa / przyszłe SDA |
| 6 | AUX2 | rezerwa / przyszłe SCL |

Proponowane gniazdo: JST-GH 6-pin 1,25 mm z zatrzaskiem. To **logiczny** pinout; fizyczne oznaczenie pinów zweryfikować według konkretnego złącza. Wszystkie GPIO pracują na 3,3 V. Nie dopuszcza się podłączania/odłączania modułów pod napięciem bez dodatkowych testów i ochrony.

## GPIO — do zatwierdzenia
Kandydaci: LED DATA GPIO4, DHT A GPIO5, DHT B GPIO6, AUX1 GPIO7, AUX2 GPIO10. **Nie traktować jako zatwierdzonego pinoutu** przed sprawdzeniem schematu konkretnej płytki ESP32-C3 SuperMini, funkcji pinów startowych, USB i rezerwacji.

Piny zajęte lub ryzykowne na SuperMini — nie używać do LED/SENSOR: GPIO2, GPIO8 (wbudowana dioda), GPIO9 (BOOT) — piny startowe; GPIO18/19 — USB; GPIO20/21 — UART.

## Przycisk RESET Wi-Fi (BOOT / GPIO9)
- Przytrzymanie **5 s** w trakcie pracy: kasowanie zapisanej sieci Wi-Fi i uruchomienie trybu konfiguracji (AP). Pozostałe ustawienia (harmonogram, moduł LED, lokalizacja) zostają.
- Firmware czyta GPIO9 z wewnętrznym pull-up dopiero po starcie. **Przytrzymanie podczas włączania zasilania** uruchamia tryb programowania ESP32 (zachowanie układu, nie błąd) — opisać w instrukcji.
- Otwór w obudowie zabezpieczony przed przypadkowym naciśnięciem (zagłębiony, na szpilkę).
- Zastępuje automatyczny timeout trybu AP: AP włącza się tylko przy braku zapisanej sieci albo po resecie przyciskiem.

## Budżet prądu
Całość 5 V / 2 A = maks. 10 W. Zostawić margines dla ESP32 podczas Wi-Fi.
- Typowe WS2812B: konserwatywnie do 60 mA na piksel przy pełnej bieli. 30 diod może pobrać 1,8 A bez ESP32 — niewłaściwe do swobodnego pełnego świecenia z 2 A.
- Niskoprądowy łańcuch WS2811 GFLAI: deklarowane około 20 mA na piksel, wymaga pomiaru na próbkach.
- Długi cienki łańcuch może wymagać dodatkowego zasilania przez spadki napięcia.
- Sygnał DATA z ESP32 (3,3 V) przechodzi **zawsze przez bufor 74HCT125 zasilany 5 V** przed wejściem LED. Połączenie bez konwertera nie jest docelowym wariantem.

## Granice — sterownik bez dedykowanych diod (decyzja 2026-10-14)
Klient może podłączyć własne diody i zasilacz, więc granice muszą wynikać ze sprzętu i firmware, nie z instrukcji.

| Parametr | Wartość | Status |
|---|---|---|
| Zasilanie | **USB-C 5 V** (bez PD), ładowarka 2–3 A | cel projektu |
| Prąd przez LED OUT | **maks. 2,5 A** (`LED_OUT_MAX_MA`) | cel projektu — sprawdzić na PCB (złącze, ścieżki, bezpiecznik) |
| Liczba pikseli | maks. 300 (`MAX_LEDS`); jasność ograniczana do budżetu | firmware |
| Prąd na piksel (ustawienie) | 10–120 mA (`MIN/MAX_MA_PER_PIXEL`) | firmware |
| Diody | adresowalne 5 V, jedna linia DATA: WS2812B, WS2811 5 V | — |

Firmware (od 2.0.0-dev.4): budżet LED nigdy powyżej 2,5 A — w API, przy wczytaniu ustawień i jeszcze raz w ograniczniku; kreator proponuje zasilacze tylko do 3 A; gdy sam prąd spoczynkowy łańcucha przekracza budżet, kolory gasną zamiast wyłączenia ogranicznika.
**Ograniczenia firmware:** ogranicznik szacuje prąd z ustawionego mA/piksel — nie mierzy go. Błędne ustawienie, zwarcie, uszkodzony łańcuch albo diody 12 V nie zostaną przez firmware zatrzymane. Dlatego wymagane zabezpieczenia sprzętowe:

- **Bezpiecznik polimerowy (PTC) na LED OUT** dobrany do 2,5 A — zwarcie i przeciążenie wyłączają wyjście, zanim ucierpi płytka.
- **Wejście zasilania USB-C** — rozwiązuje odwrotną polaryzację i zasilacz 12 V (szczegóły niżej). Przy wtyku DC byłyby potrzebne P-MOSFET i OVP.
- **ESD na DATA**, na złączu SENSOR i na VBUS (TVS).
- **Pomiar napięcia wejściowego** (dzielnik do ADC): firmware zapisuje w NVS maksymalne zmierzone napięcie i zdarzenia przepięcia — „czarna skrzynka” widoczna w panelu, dowód przy reklamacji.

**Gniazdo zasilania: USB-C (decyzja 2026-10-14).** Osobne gniazdo USB-C tylko do zasilania na płycie TerraLight (nie złącze USB płytki SuperMini — jej ścieżki nie są na 2,5 A). Odbiornik bez PD: dwa rezystory 5,1 kΩ z CC1/CC2 do masy, więc ładowarka daje wyłącznie 5 V — zasilacza 12 V nie da się podłączyć, a złącze nie pozwala odwrócić polaryzacji. Odrzucony wtyk DC 5,5/2,1: tańsze gniazdo, ale pasuje do każdego zasilacza 12/24 V i wymagałby ochrony przed przepięciem, która zjada różnicę w cenie.
- **Odczyt mocy ładowarki:** napięcie na CC (przez ADC) mówi, ile prądu deklaruje źródło — domyślne USB / 1,5 A / 3 A. Firmware może sam ustawić budżet LED i ostrzec przy słabej ładowarce. Wymaga jednego–dwóch wejść ADC (GPIO0–4) — uwzględnić w pinoucie.
- Nadal potrzebne: TVS na VBUS (ESD i przepięcia przy wpinaniu), PTC na LED OUT, pomiar napięcia wejściowego — przy USB-C głównie do wykrywania spadków napięcia (słaba ładowarka, cienki kabel).
- Do testów: tanie ładowarki i kable (spadek napięcia przy 2,5 A), ładowarki deklarujące 3 A tylko przez PD.

**Większe instalacje** (dużo diod, mocny zasilacz): diody zasilane bezpośrednio z zasilacza, przez sterownik tylko DATA i masa. Nieobsługiwane w v1 — ogranicznik zakłada, że prąd LED płynie przez płytkę.

## Testy przed PCB i sprzedażą
Polaryzacja i termika, maksymalny pobór, reset przy Wi-Fi, integralność DATA, działanie z buforem i bez, spadek napięcia na przewodach, DHT22 lub DS18B20 na obu niezależnych liniach 0,5–1 m, kondensacja, odporność obudowy, ESD. ESP32 i zasilacz na zewnątrz terrarium. RGB nie zastępuje wymaganego gatunkowo światła dziennego ani UVB.

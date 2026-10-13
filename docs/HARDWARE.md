# TerraLight — hardware v0.1

Status: projekt koncepcyjny do prototypowania, nie gotowa PCB.

## Architektura
- ESP32-C3 SuperMini, Wi-Fi 2,4 GHz; przyszły ESP-NOW.
- Jeden zasilacz 5 V / 2 A. Dwa równoległe obwody: zasilanie ESP przez wejście 5 V oraz zasilanie LED przez wyjście LED OUT. Wspólna masa.
- LED OUT: +5V / DATA / GND, jeden wymienny moduł LED na raz.
- SENSOR: port sześciopinowy 3,3 V z dwiema liniami DHT i dwiema liniami rezerwowymi.
- Brak przekaźników i obwodów 230 V w TerraLight v1.
- USB-C ESP dostępne do programowania.
- Projekt przewiduje rezystor DATA ok. 330 Ω, kondensator na LED 470–1000 µF / min. 10 V, ceramikę odsprzęgającą 100 nF i footprint opcjonalnego 74AHCT1G125 (bufor 3,3 V → 5 V).
- Dobrać zabezpieczenie zasilania (odwrotna polaryzacja, nadprąd, ESD) i szerokości ścieżek po ustaleniu PCB.

## Port SENSOR v1
| Pin | Nazwa | Rola |
|---|---|---|
| 1 | 3V3 | niskoprądowe zasilanie sond |
| 2 | GND | wspólna masa |
| 3 | DATA1 | sonda DHT22 A |
| 4 | DATA2 | sonda DHT22 B |
| 5 | AUX1 | rezerwa / przyszłe SDA |
| 6 | AUX2 | rezerwa / przyszłe SCL |

Proponowane gniazdo: JST-GH 6-pin 1,25 mm z zatrzaskiem. To **logiczny** pinout; fizyczne oznaczenie pinów zweryfikować według konkretnego złącza. Wszystkie GPIO pracują na 3,3 V. Nie dopuszcza się podłączania/odłączania modułów pod napięciem bez dodatkowych testów i ochrony.

## GPIO — do zatwierdzenia
Kandydaci: LED DATA GPIO4, DHT A GPIO5, DHT B GPIO6, AUX1 GPIO7, AUX2 GPIO10. **Nie traktować jako zatwierdzonego pinoutu** przed sprawdzeniem schematu konkretnej płytki ESP32-C3 SuperMini, funkcji pinów startowych, USB i rezerwacji.

## Budżet prądu
Całość 5 V / 2 A = maks. 10 W. Zostawić margines dla ESP32 podczas Wi-Fi.
- Typowe WS2812B: konserwatywnie do 60 mA na piksel przy pełnej bieli. 30 diod może pobrać 1,8 A bez ESP32 — niewłaściwe do swobodnego pełnego świecenia z 2 A.
- Niskoprądowy łańcuch WS2811 GFLAI: deklarowane około 20 mA na piksel, wymaga pomiaru na próbkach.
- Długi cienki łańcuch może wymagać dodatkowego zasilania przez spadki napięcia.
- Sygnał DATA 3,3 V z ESP32 przy pikselach 5 V jest tylko rozwiązaniem testowym; nie ma gwarancji poziomu HIGH. Bufor opcjonalny na PCB.

## Testy przed PCB i sprzedażą
Polaryzacja i termika, maksymalny pobór, reset przy Wi-Fi, integralność DATA, działanie z buforem i bez, spadek napięcia na przewodach, dwa DHT22 na 0,5–1 m, kondensacja, odporność obudowy, ESD. ESP32 i zasilacz na zewnątrz terrarium. RGB nie zastępuje wymaganego gatunkowo światła dziennego ani UVB.

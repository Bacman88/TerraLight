# TerraLight — płytka główna (projekt schematu v0.2)

Status: **zatwierdzone do rysowania schematu** (2026-10-15). **Schemat KiCad 9:** [schematy/kicad9/TerraLight-main](../schematy/kicad9/TerraLight-main/) — do sprawdzenia ERC w KiCad i doboru elementów. Na jej podstawie powstanie schemat i PCB w **KiCad 9**, produkcja z montażem SMD w JLCPCB. Wartości oznaczone **[do sprawdzenia]** potwierdzić w kartach katalogowych i na prototypie. Założenia: [HARDWARE.md](HARDWARE.md) (sekcja „Granice”), [SENSOR_MINI.md](SENSOR_MINI.md), [LED_MODULES.md](LED_MODULES.md).

**Decyzja (2026-10-15):** płytka od razu w wersji seryjnej z modułem **ESP32-C3-MINI-1-N4** wlutowanym w PCB (bez SuperMini). Powody: niższy koszt w serii, montaż w całości w fabryce, moduł ma certyfikat CE (klony SuperMini nie mają — ważne przy sprzedaży urządzenia radiowego w UE), jedno gniazdo USB-C do zasilania i programowania. Firmware dalej testujemy na SuperMini z buforem DIP na płytce stykowej — przydział GPIO jest ten sam.

**Zmiana (2026-10-15, wieczór):** pierwsze prototypy (5 szt.) montujemy **ręcznie**, więc na płytce jest moduł **ESP32-C3-WROOM-02-N4** — pady na krawędziach, do lutowania zwykłą lutownicą; pole GND pod spodem lutowane od dołu przez otwór metalizowany. Ten sam układ, przydział GPIO i firmware bez zmian. Elementy pasywne 0805. Przy produkcji seryjnej w fabryce można wrócić do MINI-1 (mniejszy). Szczegóły: [README projektu KiCad](../schematy/kicad9/TerraLight-main/README.md).

## Złącza (zewnętrznie bez zmian względem SuperMini)
| Złącze | Rola |
|---|---|
| **USB-C 16-pin** | zasilanie z ładowarki 5 V **i** programowanie / log przez wbudowane USB ESP32-C3 |
| **LED OUT** — gniazdo JST-VH 3-pin | +5V / DATA / GND, do 2,5 A; w zestawie kabel VH → JST-SM |
| **SENSOR** JST-GH 6-pin | Sensor MINI: 3V3, GND, DATA1, DATA2, AUX1, AUX2 (pinout w HARDWARE.md) |
| przyciski | BOOT (reset Wi-Fi 5 s / tryb programowania) i RESET — przez otwory w obudowie |

Sensor PRO łączy się bezprzewodowo (ESP-NOW) — nie wymaga złącza.

## Schemat blokowy

```
                         ┌─ D+/D- ── ESD USBLC6-2 ── GPIO19/GPIO18 (USB ESP32-C3)
USB-C 16-pin ── VBUS ── TVS ─┬── PTC 3 A ── P-MOSFET (miękki start) ──┬── 1000 µF ── LED OUT +5V
  CC1/CC2 ─ 5,1 kΩ do GND    │                    ▲                   └── 74AHCT1G125 VCC
     └─ ADC GPIO1 / GPIO3    │              GPIO20 LED_PWR_EN
                             ├── dzielnik ── ADC GPIO0 (VBUS_SENSE)
                             └── Schottky ── 100 µF ── LDO 3,3 V ──┬── ESP32-C3-MINI-1
                                                                   └── PTC 0,1 A ── SENSOR 3V3
GPIO4 ── 74AHCT1G125 ── 330 Ω ── ESD ── LED OUT DATA
GPIO5/6/7/10 ── 100 Ω ── ESD ── SENSOR DATA1/DATA2/AUX1/AUX2 (pull-upy 4,7 kΩ do 3V3)
```

Masa wspólna; prąd LED wraca do USB-C przez szeroką wylewkę, nie pod modułem ESP.

## Przydział GPIO (ESP32-C3-MINI-1)

| GPIO | Sygnał | Uwagi |
|---|---|---|
| 0 | VBUS_SENSE | ADC1_CH0, dzielnik 100 k / 47 k |
| 1 | CC1_SENSE | ADC1_CH1 |
| 3 | CC2_SENSE | ADC1_CH3 |
| 4 | LED_DATA | → bufor → LED OUT (jak w firmware) |
| 5 | DATA1 | SENSOR pin 3 |
| 6 | DATA2 | SENSOR pin 4 |
| 7 | AUX1 | SENSOR pin 5 (przyszłe SDA) |
| 10 | AUX2 | SENSOR pin 6 (przyszłe SCL) |
| 8 | STATUS_LED | dioda na płytce, aktywna stanem niskim (jak na SuperMini); pin startowy — pull-up 10 kΩ |
| 9 | BUTTON (BOOT) | przycisk do GND + pull-up 10 kΩ; pin startowy |
| 18 / 19 | USB D- / D+ | wbudowane USB Serial/JTAG |
| 20 | LED_PWR_EN | po resecie wejście + pull-down = zasilanie LED wyłączone. To też UART0 RX — pad testowy do awaryjnego programowania |
| 21 | rezerwa | UART0 TX — pad testowy |
| 2 | rezerwa | pin startowy — pull-up 10 kΩ, pad testowy |

ADC na ESP32-C3 działa pewnie tylko na GPIO0–4 (ADC1; ADC2 koliduje z Wi-Fi). Zakres przy tłumieniu 12 dB ok. 0–2,5 V. Rezystory na pinach startowych (GPIO2, 8, 9) i układ EN **[do sprawdzenia z „ESP32-C3 Hardware Design Guidelines” Espressif]**.

## Bloki schematu

### 1. USB-C: zasilanie i dane
- **J1:** gniazdo USB-C 16-pin (USB 2.0), znamionowo ≥ 3 A, z przewlekanymi zaczepami (wyrywanie kabla).
- **R1, R2:** 5,1 kΩ 1 % z CC1 i CC2 do GND → źródło daje tylko 5 V (także ładowarki PD/QC).
- **CC do ADC:** CC1 → GPIO1, CC2 → GPIO3 przez 1 kΩ. Napięcie na aktywnej linii CC (specyfikacja USB-C):

  | Napięcie CC | Źródło deklaruje |
  |---|---|
  | 0,25–0,61 V | domyślne USB (0,5–0,9 A) — np. komputer |
  | 0,70–1,16 V | 1,5 A |
  | 1,31–2,04 V | 3 A |

  Firmware bierze wyższą z dwóch linii. Przy komputerze sam obniża budżet LED do ~0,4 A.
- **D+/D-:** piny A6/B6 i A7/B7 gniazda połączone parami → **U3** USBLC6-2 (ESD) → GPIO19 (D+) / GPIO18 (D-). Krótko, para różnicowa ~90 Ω.
- **D1:** TVS na VBUS, jednokierunkowy 5 V (np. SMF5.0A) **[do doboru]**.
- **C1:** 10 µF ceramiczny + **C2:** 100 µF przy gnieździe. Duża pojemność dopiero za kluczem LED (bez udaru przy wpięciu kabla).
- **Pomiar VBUS:** R3 100 kΩ / R4 47 kΩ + C3 100 nF → GPIO0. 5,0 V → 1,60 V; zakres do ok. 7,8 V. Firmware: „czarna skrzynka” (maks. napięcie, spadki).

### 2. Tor LED (do 2,5 A)
- **F1:** PTC 1812, prąd trzymania ok. 3 A, ≥ 6 V **[do doboru]**. Firmware ogranicza do 2,5 A, PTC łapie zwarcia i błędne ustawienia.
- **Q1:** klucz P-MOSFET po stronie +5 V, R_DS(on) ≤ 30 mΩ przy V_GS = −4,5 V (przy 2,5 A ok. 0,2 W) **[do doboru]**.
  - R5 100 kΩ bramka–źródło (domyślnie wyłączony),
  - Q2 2N7002 ściąga bramkę przez R6 10 kΩ; bramka Q2 z GPIO20 przez 1 kΩ, R7 100 kΩ do GND,
  - C4 ok. 22 nF bramka–dren Q1: miękki start rzędu kilku ms **[dobrać na prototypie]**.
  - Po co: brak udaru i resetów ESP przy włączeniu; w nocy odcięcie prądu spoczynkowego diod (WS2811 ok. 1 mA/piksel → ~0,3 A przy 300 pikselach); odcięcie wyjścia przy błędzie.
- **C5:** 1000 µF / 10 V low-ESR (lub 2 × 470 µF) + **C6:** 10 µF ceramiczny przy LED OUT.
- **J2 LED OUT:** gniazdo **JST-VH 3-pin** (raster 3,96 mm, THT, z zatrzaskiem; prąd znamionowy wielokrotnie powyżej 2,5 A **[do sprawdzenia w karcie]**). Montowane przez JLCPCB (montaż THT) — płytki przychodzą gotowe, bez lutowania w domu. Do zestawu **kabel-przejściówka VH → JST-SM 3-pin żeński** (decyzja 2026-10-15) (wejście typowych pasków WS2812B), zamawiany gotowy; przekrój min. AWG 20–22. Kolejność pinów gniazda: 1 = +5V, 2 = DATA, 3 = GND **[potwierdzić]**; kolejność żył po stronie JST-SM dobrać do pasków na próbkach i opisać w KOMPATYBILNOSC.md.
- Droga USB-C → LED OUT i powrót GND: wylewka lub ścieżka ≥ 1,5 mm (miedź 35 µm).

### 3. DATA LED
- **U1:** 74AHCT1G125 (SOT-23-5), zasilany z **przełączanej** linii LED +5V, 100 nF przy VCC, /OE do GND. Wejście AHCT przyjmuje 3,3 V jako stan wysoki i nie ma diody do VCC, więc wyłączony bufor nie zasila się z GPIO **[do sprawdzenia w karcie]**.
- **R8:** 330 Ω na wyjściu → **D2:** ESD 5 V → LED OUT DATA.
- Firmware: przed wyłączeniem Q1 ustawić GPIO4 w stan niski.

### 4. Zasilanie 3,3 V i ESP32-C3
- **D3:** Schottky ≥ 1 A (np. SS14) z VBUS + **C7:** 100 µF → wejście LDO. Oddziela zasilanie ESP od spadków napięcia przy skokach prądu LED (mniej restartów przy słabej ładowarce).
- **U4:** LDO 3,3 V, ≥ 500 mA, niski spadek napięcia (np. AP2112K-3.3, SOT-23-5) **[do doboru, preferowany element „basic” w JLCPCB]**; 1 µF–10 µF na wejściu i wyjściu wg karty.
- **U5:** ESP32-C3-MINI-1-N4 — **antena wbudowana** w moduł (nie wersja -1U); obudowa plastikowa (4 MB flash — firmware z OTA ok. 1,3 MB na partycję). Przy pinie 3V3: 10 µF + 100 nF.
- **EN:** 10 kΩ do 3V3 + 1 µF do GND; **SW2 RESET** EN → GND.
- **SW1 BOOT:** GPIO9 → GND, pull-up 10 kΩ. Przytrzymanie przy włączeniu = tryb programowania; w trakcie pracy 5 s = reset Wi-Fi (bez zmian w firmware).
- **LED1 status:** 3V3 → 1 kΩ → LED → GPIO8 (aktywna stanem niskim), widoczna przez obudowę (światłowód lub otwór).
- **Antena:** moduł przy krawędzi płytki, strefa anteny bez miedzi na obu warstwach i bez elementów — wymiary wg karty modułu.

### 5. Port SENSOR (JST-GH 6-pin)
- **3V3:** przez **F2** PTC ok. 0,1 A (zwarcie w przewodzie czujnika nie wyłącza ESP).
- **DATA1, DATA2:** pull-up 4,7 kΩ do 3V3 (footprint, można nie wlutować) + 100 Ω szeregowo do GPIO.
- **AUX1, AUX2:** 100 Ω szeregowo; footprinty pull-upów I²C 4,7 kΩ — domyślnie nie wlutowane.
- **U2:** 4-kanałowa matryca ESD na DATA1, DATA2, AUX1, AUX2 **[do doboru]**.

### 6. Mechanika, programowanie, pady testowe
- USB-C, LED OUT i SENSOR na jednej krawędzi; 4 otwory M3; przyciski i dioda statusu dostępne przez obudowę.
- **Obudowa:** własna, drukowana w 3D i dopasowana do gotowej płytki. W obudowie wycięcie na gniazdo JST-VH, USB-C i SENSOR.
- 2 warstwy, 35 µm, GND na całej dolnej warstwie. Dzielnik ADC i linie CC z dala od toru LED.
- **Pierwsze wgranie firmware:** przez USB-C z komputera (PlatformIO / esptool, wbudowane USB). Dalej aktualizacje przez OTA.
- **Awaryjnie** (np. błąd na liniach USB): pady UART0 TX (GPIO21) / RX (GPIO20) / GPIO9 / EN / 3V3 / GND — przejściówka USB-UART 3,3 V.
- Pady testowe: VBUS, LED +5V, 3V3, GND, LED_DATA.
- **Pady rezerwowe na przyszłość:** GPIO2 i GPIO21 (z pull-upem na GPIO2), obok 3V3 i GND — na razie bez funkcji.

## Lista elementów (wstępna)

| Ozn. | Element | Uwagi |
|---|---|---|
| U5 | ESP32-C3-MINI-1-N4 | moduł z anteną, CE |
| U4 | LDO 3,3 V ≥ 500 mA | np. AP2112K-3.3 |
| U1 | 74AHCT1G125 | bufor DATA |
| U2 | matryca ESD 4 kanały | SENSOR |
| U3 | USBLC6-2SC6 | ESD na D+/D- |
| J1 | USB-C 16-pin, ≥ 3 A | THT zaczepy |
| J2 | JST-VH 3-pin, gniazdo THT | montaż THT w JLCPCB; + kabel VH → JST-SM w zestawie |
| J3 | JST-GH 6-pin SM | SENSOR |
| SW1, SW2 | przyciski SMD | BOOT, RESET |
| LED1 | dioda SMD | status |
| F1 | PTC 1812, ~3 A hold | tor LED |
| F2 | PTC ~0,1 A hold | 3V3 sond |
| Q1 | P-MOSFET ≤ 30 mΩ | klucz LED |
| Q2 | 2N7002 | sterowanie Q1 |
| D1 | TVS 5 V | VBUS |
| D2 | ESD 5 V | LED DATA |
| D3 | Schottky SS14 | zasilanie LDO |
| C5 | 1000 µF / 10 V low-ESR | LED OUT |
| R1, R2 | 5,1 kΩ 1 % | CC |
| R3, R4 | 100 kΩ / 47 kΩ 1 % | pomiar VBUS |
| — | 330 Ω, 4 × 100 Ω, 4 × 4,7 kΩ, 3 × 1 kΩ + 1 kΩ (LED1), 4 × 10 kΩ (EN, GPIO2/8/9), 10 kΩ, 2 × 100 kΩ; 100 nF × ~6, 1 µF, 10 µF × ~4, 100 µF × 2, 22 nF | |

## Koszt orientacyjny (2026-10-15, JLCPCB/LCSC, ~4 zł/$, bez VAT 23 %, obudowy, kabli i ładowarki)
- Elementy + PCB przy 100 szt.: ok. **17–20 zł/szt.** (moduł ESP ok. 8 zł).
- Przy małych zamówieniach dominują stałe opłaty: wysyłka 60–100 zł, uruchomienie montażu ~$8, ~$3 za każdy rodzaj elementu spoza listy „basic” JLC (u nas ok. 8–10 rodzajów). Prototyp 5–10 szt. z montażem SMD: ok. 400–500 zł całość.
- Przy doborze elementów preferować części „basic” JLCPCB — każda „extended” to dopłata za zamówienie.

## Decyzje (2026-10-15)
1. **Złącze LED OUT:** gniazdo **JST-VH 3-pin** na płytce + kabel-przejściówka VH → JST-SM **żeński** w zestawie — szczegóły w bloku 2.
2. **Klucz zasilania LED (Q1):** tak.
3. **Antena:** wbudowana w moduł (ESP32-C3-MINI-1-N4), obudowa plastikowa.
4. **GPIO2 / GPIO21:** wolne pady na przyszłość, bez funkcji.

Pozostaje do sprawdzenia na prototypie: wartości oznaczone **[do sprawdzenia]** / **[do doboru]**, kolejność żył na kablu-przejściówce (wtyk JST-SM żeński — ustalone).

## Zmiany w firmware po zatwierdzeniu
`core/pins.h`: VBUS_SENSE = 0, CC1_SENSE = 1, CC2_SENSE = 3, LED_PWR_EN = 20 (pozostałe bez zmian). Sterowanie kluczem LED (włączanie przed pierwszą klatką ≠ czerni, wyłączanie po ok. 1 min czerni, LED_DATA niski przed wyłączeniem), odczyt mocy źródła z CC (komputer → niski budżet LED), „czarna skrzynka” napięcia.

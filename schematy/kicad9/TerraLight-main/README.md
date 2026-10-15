# TerraLight — płytka główna, projekt KiCad 9

Schemat według [docs/PCB.md](../../../docs/PCB.md) (v0.2, 2026-10-15). Otwórz `TerraLight-main.kicad_pro` w KiCad 9.

## Co jest w środku
| Plik | |
|---|---|
| `TerraLight-main.kicad_sch` | schemat, jeden arkusz A3, 6 bloków jak w PCB.md |
| `TerraLight.kicad_sym` | biblioteka symboli projektu (podpięta przez `sym-lib-table`) |
| `TerraLight.pretty/` | footprint ESP32-C3-WROOM-02 (Espressif, CC-BY-SA 4.0) + otwór w padzie GND do lutowania od spodu |
| `BOM_prototyp_5szt.csv` | lista zakupów na 5 płytek: numery producentów (MPN) do wyszukania w TME / LCSC / Mouser, ilości z zapasem |

Projekt nie wymaga dodatkowych bibliotek symboli. Footprinty poza modułem ESP32 pochodzą ze standardowych bibliotek KiCad.
Model 3D modułu ESP32 jest dostępny po zainstalowaniu biblioteki Espressif z menedżera wtyczek (PCM) KiCad.

## Jak powstał i co sprawdzono
Schemat wygenerowano skryptem, bez KiCad (w środowisku nie było dostępu do `kicad-cli`):
- symbole standardowe są kopiami bibliotek KiCad 9; ESP32-C3-WROOM-02 pochodzi z oficjalnej biblioteki Espressif. USBLC6-2SC6, 74AHCT1G125 i SRV05-4 narysowano w tym samym stylu.
- każdy pin ma krótki odcinek przewodu zakończony etykietą sieci lub symbolem zasilania. Nie ma skrzyżowań ani rozgałęzień, które mogłyby się „nie złapać”.
- połączenia sprawdzono niezależnym skryptem (geometria pinów zweryfikowana na prawdziwych schematach KiCad 9): **53 sieci, 0 niepodłączonych pinów, 0 wiszących przewodów**. Każda sieć zgadza się z listą zamierzonych połączeń.
- składnia pliku została porównana ze schematami zapisanymi przez KiCad 9 i nie zawiera nieznanych elementów.

**Po pierwszym otwarciu w KiCad:**
1. *Inspekcja → Electrical Rules Checker (ERC)* — powinien przejść bez błędów. Ewentualne ostrzeżenia wrzucić do zgłoszenia.
2. *Narzędzia → Przypisz footprinty* — przejrzeć proponowane footprinty (kolumna Footprint), szczególnie te oznaczone niżej.
3. Zapisać projekt (KiCad może zaktualizować format pliku).

## Moduł: ESP32-C3-WROOM-02 (decyzja 2026-10-15)
Prototypy lutujemy ręcznie, więc zamiast ESP32-C3-MINI-1 (pady tylko pod spodem, wymaga hot aira) jest **ESP32-C3-WROOM-02-N4**:
- pady **na krawędziach modułu** (półotwory) — lutuje się je zwykłą lutownicą,
- pole GND pod modułem ma w footprincie **otwór metalizowany** — cynę podaje się lutownicą od spodu płytki, aż wypełni otwór,
- ten sam układ ESP32-C3, 4 MB flash, antena PCB na krawędzi modułu; moduł większy (18 × 20 mm).

Przydział GPIO bez zmian (tabela w PCB.md). Firmware nie wymaga zmian. W wersji seryjnej (montaż w fabryce) można wrócić do MINI-1.

## Założenia przyjęte przy rysowaniu
Elementy w PCB.md oznaczone **[do doboru]** mają na schemacie typowe wartości robocze. Szczegóły są w polu **Uwagi** każdego elementu (pole ukryte, widoczne we właściwościach).

| Element | Na schemacie | Do decyzji |
|---|---|---|
| J2 LED OUT | JST **S3P-VH kątowe** (kabel wychodzi z krawędzi płytki, jak USB-C i SENSOR) | sprawdzić nazwę footprintu w bibliotece KiCad |
| J1 USB-C | footprint HRO TYPE-C-31-M-12 (16-pin, zaczepy THT) | potwierdzić prąd ≥ 3 A; ekran (S1) połączony z GND |
| D1 TVS VBUS | SMF5.0A, SOD-123F | |
| F1 / F2 | PTC 1812 ~3 A / PTC 0805 ~0,1 A | konkretne typy |
| Q1 | P-MOSFET SOT-23, pinout 1=G 2=S 3=D | typ ≤ 30 mΩ; sprawdzić pinout wybranego tranzystora |
| C4 | 22 nF | dobór na prototypie (miękki start) |
| C5 | 1000 µF / 10 V, CP_Elec_10x10.5 | albo 2 × 470 µF |
| U2 | SRV05-4 (SOT-23-6, VCC = 3V3_SENSOR) | inna matryca ESD 4 kanały |
| D2 | dioda ESD 5 V, SOD-323 | |
| U4 | AP2112K-3.3 | LDO z listy „basic” JLCPCB |
| SW1, SW2 | footprint SW_SPST_TL3342 | dopasować do przycisków dostępnych w JLCPCB i obudowy |
| Pasywne | **0805** (decyzja 2026-10-15: prototypy lutujemy sami — pasta + hot air / lutownica) | |

Pozostałe decyzje projektowe:
- **USB D+/D-:** jedna sieć `USB_DP` / `USB_DN` od złącza przez USBLC6-2 (pady 1↔6, 3↔4) do ESP. Na PCB ścieżka przechodzi kolejno przez pady.
- **Pull-upy DATA1/DATA2** (4,7 kΩ) do `3V3_SENSOR` (za bezpiecznikiem F2), po stronie złącza. Rezystory 100 Ω szeregowo od strony ESP.
- **Pady programowania** J4 (3V3, TXD0, RXD0/LED_PWR_EN, BOOT, EN, GND) i **pady rezerwowe** J5 (3V3, GPIO2, GPIO21, GND) jako footprint listwy 2,54 mm, oznaczone DNP (bez wlutowanej listwy).
- **PWR_FLAG** na sieciach VBUS, +5V_LED, VLDO_IN, 3V3_SENSOR i GND — dla ERC.
- Każdy element ma pole **MPN** (numer producenta). Pozycje w nawiasach [ ] to specyfikacja do dobrania. Kody sklepów (TME/LCSC) nie są przypisane — wyszukiwać po MPN.
- **Prototypy (5 szt.) montujemy sami**, wszystko SMD: pasta przez szablon + hot air (ESP, USB-C), resztę można lutownicą. Kolejność: najpierw SMD, na końcu przewlekane (JST-VH).

## Następne kroki
1. ERC i przegląd schematu w KiCad.
2. Dobór konkretnych elementów (kody LCSC) i aktualizacja tabeli w PCB.md.
3. Klasy sieci dla toru zasilania LED (≥ 1,5 mm), potem PCB: moduł przy krawędzi ze strefą anteny bez miedzi, USB-C / LED OUT / SENSOR na jednej krawędzi, 4 otwory M3.

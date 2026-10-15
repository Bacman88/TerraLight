# TerraLight MINI — płytka dla początkujących (KiCad 9)

Najprostsza własna płytka TerraLight: gotowy **ESP32-C3 SuperMini** wkładany w dwie listwy żeńskie, a reszta to **duże elementy przewlekane (THT)**. Nie potrzeba pasty, hot aira ani lutowania modułu ESP. Ten sam układ co [instrukcja breadboard](../../breadboard/INSTRUKCJA.md), przeniesiony na płytkę i uzupełniony o bezpiecznik, diodę i zaciski śrubowe.

Otwórz `TerraLight-mini.kicad_pro` w KiCad 9. Podgląd schematu (uproszczony render): [podglad.png](podglad.png).

Płytka główna z modułem wlutowanym w PCB ([TerraLight-main](../TerraLight-main/)) zostaje docelową wersją seryjną. MINI służy do nauki, testów firmware w terrarium i pierwszych egzemplarzy.

## Co jest na płytce (16 elementów do przylutowania)
| Oznaczenie | Element | Po co |
|---|---|---|
| U1 | ESP32-C3 SuperMini w 2 listwach żeńskich 1×8 | mózg; ma już stabilizator 3,3 V, przyciski BOOT/RESET, diodę i USB-C do programowania |
| J1 | zacisk śrubowy 2-pin | wejście 5 V z modułu gniazda USB-C (moduł przykręcony w obudowie) |
| F1 | bezpiecznik polimerowy 3 A | zwarcie na diodach LED wyłącza tylko LED-y; sam się „naprawia” po ostygnięciu |
| C1 | elektrolit 1000 µF 16 V | łagodzi skoki prądu diod |
| J2 | zacisk śrubowy 3-pin | LED OUT: 1 = +5 V, 2 = DATA, 3 = GND |
| D1 | dioda 1N5819 | zasila SuperMini i nie pozwala, by płytka „wpychała” prąd do komputera przy podpiętym kablu USB |
| C3 | elektrolit 100 µF 16 V | podtrzymuje ESP przy skokach prądu |
| R2, R3 | 100 kΩ i 47 kΩ | pomiar napięcia zasilacza (GPIO0) — „czarna skrzynka” w firmware |
| U2 | 74HCT125 w podstawce DIP-14 | wzmacnia sygnał DATA z 3,3 V do 5 V |
| C2 | 100 nF | odsprzęgnięcie U2 — lutować tuż przy nim |
| R1 | 330 Ω | rezystor na linii DATA |
| J3 | gniazdo JST-XH 6-pin | port SENSOR: 1 = 3V3, 2 = GND, 3 = DATA1, 4 = DATA2, 5 = AUX1, 6 = AUX2 |
| R4, R5 | 4,7 kΩ | podciągnięcie linii czujników DHT22 / DS18B20 |

Lista zakupów na 5 płytek: [BOM_prototyp_5szt.csv](BOM_prototyp_5szt.csv). Moduł gniazda USB-C musi mieć **rezystory 5,1 kΩ na liniach CC** (w opisie: „5.1k”, „CC resistors”, „zasilanie 5 V z ładowarki USB-C”) — bez nich ładowarka USB-C nie poda napięcia.

## Przydział GPIO (jak w firmware `core/pins.h` i płytce głównej)
| GPIO | Sygnał |
|---|---|
| 4 | LED DATA → 74HCT125 → 330 Ω → LED OUT |
| 5 / 6 | DATA1 / DATA2 (czujniki) |
| 7 / 10 | AUX1 / AUX2 (rezerwa, I²C) |
| 0 | pomiar napięcia zasilacza (dzielnik 100k/47k) |
| 8 / 9 | dioda i przycisk BOOT — już na SuperMini |

Firmware nie wymaga zmian. Odczyt CC (moc ładowarki) i klucz zasilania LED z płytki głównej tutaj nie występują.

## Zanim zamówisz płytki — sprawdź SuperMini
Klony SuperMini różnią się między sobą. Footprint zakłada: rzędy pinów **15,24 mm** od siebie, raster 2,54 mm, widok z góry z USB-C u góry: **prawa kolumna 5V, GND, 3V3, GPIO4, 3, 2, 1, 0**, lewa **GPIO5, 6, 7, 8, 9, 10, 20, 21**.
1. Zmierz suwmiarką odległość między rzędami pinów swojego SuperMini.
2. Porównaj nadruk na płytce z powyższą kolejnością.
3. Najlepiej: wydrukuj widok PCB w skali 1:1 i przyłóż SuperMini z goldpinami.

## Kolejność lutowania i uruchamiania
1. **Najniższe elementy najpierw:** rezystory, dioda D1 (pasek = katoda, zgodnie z nadrukiem), C2.
2. Podstawka DIP-14 (wycięcie zgodnie z nadrukiem), listwy żeńskie (włóż w nie SuperMini na czas lutowania — wtedy staną prosto), gniazdo J3.
3. Zaciski J1, J2, bezpiecznik F1, elektrolity C1 i C3 (**dłuższa nóżka = plus**, pasek na obudowie = minus).
4. **Test bez ESP i bez 74HCT125:** podłącz zasilanie, zmierz miernikiem: J1 ≈ 5 V, J2 pin 1 ≈ 5 V, pin 14 podstawki ≈ 5 V, katoda D1 ≈ 4,7 V.
5. Odłącz zasilanie, włóż 74HCT125 (wycięcie jak w podstawce) i SuperMini (USB-C w stronę nadruku „USB-C”).
6. Wgraj firmware przez USB-C SuperMini, podłącz kilka diod do J2 i sprawdź ograniczenie jasności w panelu.

## Jak powstał i co sprawdzono
Wygenerowany tym samym skryptem co płytka główna: każdy pin ma odcinek przewodu z etykietą sieci, połączenia sprawdzone niezależnie (**14 sieci, 0 niepodłączonych pinów**, zgodność z listą zamierzonych połączeń, składnia porównana z plikami KiCad 9). Symbole SuperMini i 74HCT125 oraz footprint gniazda SuperMini narysowano dla tego projektu. Pozostałe footprinty pochodzą ze standardowych bibliotek KiCad.

**Po otwarciu w KiCad:** ERC (*Inspekcja → Electrical Rules Checker*), potem *Narzędzia → Aktualizuj PCB ze schematu* i rozmieszczanie.

## Wskazówki do PCB
- Płytka 2-warstwowa, wszystko THT; spodnia warstwa — wylewka GND.
- Ścieżki **J1 → F1 → C1 → J2** i masa do J2: szerokie, min. **2 mm** (do 2,5 A).
- **Antena SuperMini** (koniec bez USB-C) przy krawędzi płytki lub poza nią, bez miedzi pod nią. Gniazdo USB-C SuperMini dostępne z krawędzi — do programowania.
- C2 tuż przy pinach 14 i 7 U2; R1 blisko J2.
- Zaciski J1, J2 i gniazdo J3 na jednej krawędzi (kable wychodzą z jednej strony obudowy).

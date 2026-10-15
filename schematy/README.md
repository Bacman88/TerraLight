# TerraLight — schematy i instrukcje

Dokumentacja robocza przygotowana 15.10.2026. **Nie są to projekty produkcyjne.**

## Warianty

### 1. `breadboard/` — testy na ESP32-C3 SuperMini
Najprostszy wariant: USB 5 V → SuperMini + 5 V LED, wspólne GND; GPIO4 → bufor 74HCT125 (Pixel Boost) → 330 Ω → DIN LED. Kondensator 100 nF przy buforze, 470–1000 µF na zasilaniu LED. Czujniki DHT22/DS18B20 są opcjonalne (GPIO5/GPIO6).

Instrukcja w [breadboard/INSTRUKCJA.md](breadboard/INSTRUKCJA.md).

### 2. `kicad9/` — płytka seryjna (projekt inżynierski)
Opis docelowej wersji w [docs/PCB.md](../docs/PCB.md). Wariant seryjny stosuje **ESP32-C3-MINI-1-N4 wlutowany w PCB**, USB-C z USB-Serial/JTAG, zabezpieczenia, PTC, klucz LED i port SENSOR. To **inna konstrukcja** niż prototyp na SuperMini.

Projekt KiCad 9 (schemat v0.2): [kicad9/TerraLight-main](kicad9/TerraLight-main/) — opis i założenia w jego README.
Lista koniecznych przeglądów: [kicad9/WERYFIKACJA.md](kicad9/WERYFIKACJA.md).

### 3. `kicad9/TerraLight-mini` — płytka dla początkujących
ESP32-C3 SuperMini w listwach żeńskich + elementy przewlekane: układ z breadboardu na własnej płytce, z bezpiecznikiem LED, diodą od zasilania zwrotnego, zaciskami śrubowymi i gniazdem SENSOR. 16 elementów do przylutowania, bez SMD. Opis, kolejność lutowania i testy: [kicad9/TerraLight-mini](kicad9/TerraLight-mini/).

### 4. `easyeda/` — minimalny schemat
Eksperymentalny wariant dla EasyEDA Standard. Wymaga potwierdzenia poprawności importu i pinoutu płytki SuperMini. Nie używać bez sprawdzenia.

## Ważne
- **GPIO to nie zasilanie LED!** 5 V LED pobieramy z linii USB/VBUS tylko po sprawdzeniu obciążalności konkretnego SuperMini; 3V3 nie służy do zasilania WS2812B.
- 6 pikseli WS2812B może pobierać do ok. 360 mA przy pełnej bieli plus prąd ESP32. Wstępne testy: ograniczona jasność i kontrola temperatury/spadku napięcia.
- Pixel Boost poprawia poziom sygnału DATA 3,3 → 5 V, **nie ogranicza prądu** LED.
- Wszystkie układy mają wspólną masę.
- Oznaczenia pinów i polaryzację kondensatora sprawdzić przed podłączeniem.
- Pliki wizualne PDF/PNG oraz niezweryfikowane pliki CAD były udostępnione jako załączniki rozmowy; **nie są jeszcze zapisane w tym repo**. Nie uznawać tej dokumentacji za zastępstwo tych plików.

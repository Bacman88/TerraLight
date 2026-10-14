# TerraLight — zasilanie i zgodne diody LED

> **SZKIC (2026-10-14)** — dokument dla klienta. Wartości oznaczone **[do pomiaru]** potwierdzić na gotowej PCB i próbkach diod przed sprzedażą. Część „Gwarancja” wymaga sprawdzenia przez prawnika. Źródło liczb: [HARDWARE.md](HARDWARE.md), sekcja „Granice”.

Ten dokument mówi, co możesz bezpiecznie podłączyć do sterownika TerraLight. Najprościej i najpewniej jest użyć **zestawu TerraLight** — sterownika z dedykowanym modułem LED, który jest sprawdzony i od razu skonfigurowany. Jeśli podłączasz własne diody, trzymaj się poniższych zasad.

## Zasilanie

TerraLight zasilasz **ładowarką USB-C**, taką jak do telefonu.

| | |
|---|---|
| Napięcie | 5 V — ładowarka USB-C podaje je sama, nic nie ustawiasz |
| Prąd ładowarki | zalecane **3 A**, minimum 2 A |
| Kabel | USB-C – USB-C albo USB-A – USB-C, krótki i gruby (np. „3 A”) |

- Szybkie ładowarki (PD, QC) są w porządku — TerraLight zawsze pobiera z nich tylko 5 V.
- Słaba ładowarka albo długi, cienki kabel to najczęstsza przyczyna mrugania diod i restartów sterownika. W razie problemów najpierw wymień kabel i ładowarkę.
- Mocniejsza ładowarka niż 3 A nie da więcej światła — sterownik ogranicza prąd diod do **2,5 A** **[do pomiaru]**.

## Zgodne diody

Podłączysz **adresowalne diody 5 V z jedną linią danych**:

| Układ | Przykład | Uwagi |
|---|---|---|
| **WS2812B** (5 V) | paski, pierścienie, pojedyncze piksele | ok. 60 mA na diodę przy pełnej bieli |
| **WS2811 5 V** | łańcuchy „pixel fairy lights” | zwykle 12–20 mA na diodę **[do pomiaru na próbkach]** |

**Nie podłączaj:**
- diod i pasków **12 V ani 24 V** (także WS2811 w wersji 12 V) — na 5 V nie zadziałają poprawnie;
- zwykłych pasków **RGB z czterema przewodami** (bez układu w każdej diodzie) — to nie są diody adresowalne;
- diod **RGBW** (np. SK6812 RGBW) — obecne oprogramowanie obsługuje tylko RGB;
- niczego do gniazda SENSOR poza czujnikami TerraLight.

Złącze LED: **+5 V / DATA / GND** **[typ złącza i kolejność pinów — do ustalenia]**. Gotowe przewody różnych producentów mają różną kolejność — sprawdź opis, zanim podłączysz.

## Ile diod?

Możesz podłączyć **do 300 diod**. Sterownik sam pilnuje, żeby nie przekroczyć możliwości ładowarki — jeśli diod jest dużo, przy jasnych kolorach przyciemni je. Tabela pokazuje, ile diod świeci **pełną bielą bez przyciemniania**:

| Ładowarka | WS2812B (60 mA) | WS2811 fairy (20 mA) |
|---|---:|---:|
| 5 V / 2 A | ok. 25 | ok. 75 |
| 5 V / 3 A | ok. 40 | ok. 125 |

Sceny dzienne rzadko używają pełnej bieli, więc w praktyce możesz mieć więcej diod bez widocznego przyciemnienia.

W panelu ustawiasz liczbę diod, układ i ładowarkę (kreator pierwszego uruchomienia albo **Moduł LED**). Ustaw prąd na diodę zgodnie z prawdą — sterownik na jego podstawie liczy pobór. Za niska wartość nie da więcej światła, tylko przeciąży ładowarkę.

## Montaż

- Sterownik i ładowarkę montuj **na zewnątrz** terrarium / akwarium — z dala od wilgoci i skraplającej się pary.
- Do środka wchodzą tylko diody — w wersji wodoodpornej (IP67/IP68) tam, gdzie jest wilgoć lub woda.
- Podłączaj i odłączaj diody i czujniki **przy odłączonym zasilaniu**.
- Diody RGB nie zastępują oświetlenia dziennego ani UVB wymaganego przez zwierzęta i rośliny — TerraLight to oświetlenie nastrojowe i symulacja cyklu dnia.

## Gwarancja **[do weryfikacji prawnej]**

Gwarancja obejmuje sterownik pracujący zgodnie z tym dokumentem. Nie obejmuje uszkodzeń spowodowanych przez:
- podłączenie diod lub urządzeń niezgodnych z sekcją „Zgodne diody”;
- zasilanie inne niż USB-C 5 V;
- zwarcie lub uszkodzenie przewodów diod;
- wilgoć — montaż sterownika wewnątrz terrarium / akwarium;
- przeróbki sterownika.

Sterownik zapisuje najwyższe zmierzone napięcie zasilania i zdarzenia przeciążenia **[funkcja do wdrożenia w hardware v1]**. Uprawnienia konsumenta wynikające z przepisów o niezgodności towaru z umową pozostają bez zmian.

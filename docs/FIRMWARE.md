# TerraLight — firmware v2.0 (ustalenia)

Status: zbiór decyzji przed pisaniem kodu. Pełna specyfikacja modułów i API panelu — w przygotowaniu.

Baza kodu: TerraLight v1.0.0 (scheduler, renderer słońca, geometria procentowa) + komponenty WeatherLight v6.3.0 (OTA z GitHuba, captive portal, NVS, panel WWW / REST, Open-Meteo, lokalne wyliczanie wschodu i zachodu).

## Tryby — dwie niezależne osie
**Wzór świecenia**
- wędrujące słońce (hotspot: wschód → zenit → zachód, kąt na pierścieniu, pozycja na linii),
- wszystkie diody w bieżącym kolorze słońca,
- co druga / co N-ta dioda (rzadsze, łagodniejsze światło),
- stały kolor.

**Źródło harmonogramu**
- stałe godziny ustawione przez użytkownika,
- rzeczywisty wschód i zachód dla lokalizacji: Open-Meteo, z lokalnym wyliczeniem astronomicznym jako zapas (działa bez internetu, wymaga tylko aktualnego czasu),
- opcjonalne przesunięcie / skrócenie względem rzeczywistego słońca.

## Czas i praca bez Wi-Fi
- NTP, gdy jest sieć; strefa czasowa jako reguła POSIX przez `configTzTime()` (domyślnie `CET-1CEST,M3.5.0,M10.5.0/3`), wybór strefy w panelu.
- Ustawienie czasu z telefonu/przeglądarki w panelu.
- Brak znanego czasu po zaniku zasilania → tryb awaryjny „bezpieczna noc”, nigdy losowe słońce w nocy.
- (do decyzji hardware) opcjonalny RTC na AUX1/AUX2 (I²C).

## Wi-Fi i tryb AP
- AP tylko przy braku zapisanej sieci albo po przytrzymaniu przycisku BOOT/GPIO9 przez 5 s (patrz HARDWARE.md). Bez automatycznego timeoutu.
- Gdy zapisana sieć jest chwilowo niedostępna (router wyłączony) — ponawianie połączenia w tle, **bez** przechodzenia w AP.
- AP działa **nieblokująco**: cykl dnia i LED pracują dalej podczas konfiguracji.
- Wspólne hasło AP dla wszystkich egzemplarzy (AP służy wyłącznie do podania sieci domowej).

## Moduły LED
- Profil modułu: liczba pikseli, typ układu (WS2812B / WS2811), kolejność kolorów, geometria (pierścień / łuk / linia), limit prądu na piksel.
- Kolejność kolorów domyślnie **RGB**; do weryfikacji na próbkach, zmienna w profilu.
- Programowy limiter prądu wg profilu i budżetu zasilacza 5 V / 2 A.

## Sensor MINI (2 × DHT22)
- Odczyt przez sprzętowy odbiornik RMT ESP32-C3 zamiast programowego odmierzania czasu — przerwania Wi-Fi na jednordzeniowym C3 nie psują odczytu.
- Retry, odstęp min. 2 s na czujnik, znacznik błędu / braku czujnika / złej sumy kontrolnej; nieaktualne dane nigdy nie są pokazywane jako bieżące.

## Znane błędy do poprawy (z v1.0.0 / WeatherLight v6.3.0)
- [ ] **Strefa czasowa:** `configTime(3600, 3600, …)` daje UTC+2 przez cały rok — zimą zegar spieszy się o 1 h. Dotyczy TerraLight v1.0.0 **i WeatherLight v6.3.0**. Poprawka: `configTzTime()` z regułą POSIX.
- [ ] **Blokujący captive portal:** pętla `while (apMode)` zatrzymuje cykl dnia, LED gasną po 60 s.
- [ ] Captive portal używa stałej `NUMPIXELS` zamiast skonfigurowanej liczby LED.
- [ ] Pinout v1 (LED na GPIO2 — pin startowy, przycisk na GPIO5) nieaktualny względem hardware v0.1.

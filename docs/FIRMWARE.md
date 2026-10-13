# TerraLight — firmware v2.0 (ustalenia)

Status: zbiór decyzji przed pisaniem kodu. Pełna specyfikacja modułów i API panelu — w przygotowaniu.

Baza kodu: TerraLight v1.0.0 (scheduler, renderer słońca, geometria procentowa) + wybrane komponenty WeatherLight v6.3.0.

## Środowisko
- **PlatformIO** (VS Code / VSCodium), ESP32 Arduino core, natywne USB SuperMini (USB CDC On Boot). Wgrywanie po USB jak w Arduino IDE, kolejne wersje przez OTA.
- Przed wydaniem kod kompilowany kontrolnie przez PlatformIO CLI.

## Struktura trybów
Jedno ustawienie bazowe (moduł) i trzy niezależne warstwy: **sterowanie** (kiedy), **światło dzienne** (co), **noc** (po zmierzchu). Na wierzchu — **nadpisania chwilowe** (obserwacja, demo).

### 0. Profil urządzenia i moduł — ustawiane raz
**Profil: Terra / Aqua.** Wybór na starcie konfiguracji; określa dostępne sceny, presety, domyślne wartości i obsługiwany czujnik. Opisane niżej warstwy 1–4 dotyczą trybu **Terra**; tryb **Aqua** — osobna sekcja.

Liczba LED, typ układu (WS2812B / WS2811), kolejność kolorów (domyślnie **RGB**, do weryfikacji na próbkach), kształt (pierścień / łuk / linia), globalny limit jasności, limiter prądu wg profilu i budżetu zasilacza 5 V / 2 A.

### 1. Sterowanie — kiedy świeci
- **Ręcznie:** wł./wył.
- **Harmonogram:**
  - stałe godziny,
  - astronomiczny — rzeczywisty wschód i zachód dla **lokalizacji biotopu** (szerokość i długość geograficzna, niezależne od miejsca instalacji; strefa czasowa pozostaje domowa). wyliczane **lokalnie** (bez internetu, dokładność ±1–2 min),
  - długość świtu i zmierzchu, przesunięcie względem słońca, minimalna i maksymalna długość dnia.
- **Sezonowość / zimowanie:** płynne skracanie dnia i obniżanie jasności w wybranym okresie (europejskie mrówki, część agam). W trybie astronomicznym zmiana pór roku wynika z lokalizacji biotopu.

### 2. Światło dzienne — co świeci
**A. Stałe:** kolor, jasność, gęstość (wszystkie / co 2. / co N-ta dioda).

**B. Dynamiczne — sceny:**
1. **Dzień na całym module** — wschód → dzień → zachód na wszystkich diodach jednocześnie; gęstość (co N-ta), paleta (ciepła, neutralna, pustynna, własna).
2. **Wędrujące słońce** — liczba LED tworzących słońce, dioda startowa, kierunek (na pierścieniu zgodnie / przeciwnie do zegara), **zakres trasy ustawiany przez użytkownika**, tło poza słońcem.
3. **Rośliny** — osobny tryb: mieszanka czerwieni i niebieskiego z suwakiem proporcji, własny fotoperiod. Oświetlenie dekoracyjne / wspomagające — nie zastępuje lampy do uprawy (patrz uwagi).
4. **Pochmurna dżungla** — rozproszone, chłodniejsze, lekko zielonkawe światło z wolno przesuwającymi się chmurami; opcjonalnie przelotny deszcz (ściemnienie, chłodniejszy kolor) oraz **burza z błyskami — domyślnie wyłączona** (np. paludarium).

### 3. Noc — po zmierzchu
Wyłączone · poświata księżyca (stała) · **rzeczywiste fazy księżyca** (liczone lokalnie) · czerwone światło obserwacyjne.

### 4. Nadpisania chwilowe
- **Tryb obserwacji:** czerwone światło na wybrany czas (np. 15 min), potem automatyczny powrót do harmonogramu. Czerwień nie stresuje mrówek — kluczowe dla formikariów. Oparty o `timer_mode` z WeatherLight.
- **Demo / podgląd:** cała doba przyspieszona do ok. 2 min — testy i prezentacja.

## Tryb Aqua
Osobny profil dla akwarium — tania alternatywa dla sterowników oświetlenia za kilkaset złotych. Wspólne z Terra: moduł, sterowanie (ręczne / harmonogram / lokalizacja biotopu / sezonowość), łagodny start, demo, tryb pracy bez Wi-Fi.

**Sceny dzienne**
- Dzień na całym module oraz wędrujące słońce — jak w Terra.
- **Refleksy wody (kaustyki)** — falujące rozbłyski jak światło słońca przechodzące przez powierzchnię wody; nakładane na scenę dzienną, z regulacją intensywności i prędkości.
- **Biotopy (palety + efekty):** Amazonka / czarna woda (przygaszone, bursztynowo-herbaciane), Malawi / Tanganika (jasne, czyste, lekko niebieskie, z refleksami), rafa (dominujący niebieski w stylu aktinicznym — dekoracyjnie).

**Harmonogram**
- **Sjesta:** przygaszenie światła w środku dnia na ustawiony czas (ograniczanie glonów w akwariach roślinnych).
- Domyślnie dłuższy świt i zmierzch niż w Terra — ryby nie mają powiek; nagłe zapalenie światła płoszy je i grozi uderzeniem o szybę.

**Noc:** niebieska poświata księżyca, rzeczywiste fazy księżyca, wyłączone.

**Nadpisania chwilowe:** **tryb serwisowy** — pełna biel na czas karmienia / sprzątania, potem automatyczny powrót do harmonogramu. Demo jak w Terra.

**Czujnik:** wodoodporna sonda temperatury **DS18B20 (1-Wire)** podłączana w miejsce DHT22 na DATA1 / DATA2 — odczyt, wykres 24 h, progi alarmowe (za zimno / za ciepło). Typ czujnika na kanale wybierany w ustawieniach (DHT22 / DS18B20 / brak). AUX1/AUX2 zostają wolne dla I²C.

**Montaż (do instrukcji):** moduł LED nad pokrywą akwarium, IP67/68, sterownik i zasilacz z dala od wody i skroplin. RGB nie zastępuje lampy dla roślin ani koralowców.

## Zachowanie urządzenia
- **Łagodny start:** po starcie lub zaniku zasilania światło dochodzi do bieżącej fazy w ok. 1 min — nigdy nagłe pełne światło.
- Wszystkie przejścia między trybami i scenami płynne.

## Biotopy i zaprogramowana pogoda
Biotop (pustynia, step, dżungla, las umiarkowany, paludarium, Amazonka, Malawi, rafa…) to preset: paleta barw, sceny i **własny, zaprogramowany „klimat”** — bez danych pogodowych z internetu.
- Każdy biotop ma prawdopodobieństwa zdarzeń: zachmurzenie, przelotny deszcz, burza (tylko jeśli włączona), dni pochmurne / słoneczne; oraz ich długość i częstotliwość.
- **Randomizacja** z ziarnem zależnym od daty: każdy dzień inny, ale ten sam dzień w podglądzie / demo wygląda tak samo.
- Ograniczenia dobrostanu: brak burz w nocy i podczas świtu, limit zdarzeń na dobę, płynne przejścia, możliwość wyłączenia losowości.

**Źródło klimatu** (wybór w biotopie):
1. **Zaprogramowany** — losowy wg biotopu, bez internetu. **Domyślny.**
2. **Na żywo — lokalizacja biotopu** — rzeczywista pogoda z Open-Meteo dla współrzędnych biotopu (np. Alice Springs dla agamy, Manaus dla akwarium amazońskiego).
3. **Na żywo — mój dom** — pogoda z lokalizacji instalacji („pada za oknem — pada nad akwarium”). Przy biotopach pustynnych / słonecznych panel ostrzega, że lokalna pogoda (np. polska jesień) może dawać zbyt mało światła.

Pogoda na żywo przechodzi przez **te same zabezpieczenia** co losowanie: brak burz w nocy i o świcie, minimalna jasność dnia dla biotopu, limit pochmurnych dni pod rząd. Rzeczywista pogoda decyduje *kiedy* — zabezpieczenia decydują *jak mocno*. Zapytanie co 15–30 min; bez internetu automatyczny powrót do klimatu zaprogramowanego.

## Internet — czego potrzebujemy
- **NTP** (`pool.ntp.org`) — czas. Jedyna zależność istotna dla działania; bez sieci czas ustawiany z telefonu.
- **GitHub Releases API** — OTA (sprawdzanie raz na dobę; limit 60 zapytań/h bez klucza). Repo / release'y publiczne.
- Wschód, zachód i fazy księżyca liczone lokalnie; strefy czasowe z wbudowanej listy reguł POSIX.
- **Open-Meteo** — wyłącznie dla opcjonalnej pogody na żywo (bez klucza API); urządzenie działa w pełni bez niego.

## Wybór lokalizacji biotopu
- **Mapa** (Leaflet + kafelki OpenStreetMap, jak w WeatherLight): klik lub przeciągnięcie znacznika; opcjonalnie wyszukiwarka miejsc (Open-Meteo Geocoding). Mapa ładowana **przez przeglądarkę telefonu** z internetu — działa, gdy telefon ma dostęp do sieci.
- Zawsze dostępne ręczne pola szerokość / długość geograficzna oraz lista gotowych lokalizacji biotopów — gdy mapa się nie wczyta (tryb AP, brak internetu).
- Geolokalizacja GPS telefonu niedostępna (przeglądarka wymaga HTTPS, panel działa po HTTP).
- Zasada: **podstawowy panel nie korzysta z CDN** — wszystkie skrypty i style wbudowane w firmware; zasoby z internetu tylko dla funkcji opcjonalnych (mapa) z działającą alternatywą.

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

## Czujniki — kanały DATA1 / DATA2
Każdy kanał konfigurowany **niezależnie** od drugiego i od profilu Terra/Aqua: **DHT22 / DS18B20 / brak**. Profil ustawia tylko wartość domyślną.

| Zastosowanie | DATA1 | DATA2 |
|---|---|---|
| Terrarium, formikarium (domyślnie Terra) | DHT22 — strefa ciepła | DHT22 — strefa chłodna |
| Terrarium z miejscem wygrzewania (np. agama) | DHT22 — powietrze | DS18B20 — powierzchnia pod lampą (kamień / podłoże) |
| Paludarium | DHT22 — powietrze | DS18B20 — woda |
| Akwarium (domyślnie Aqua) | DS18B20 (jedna lub kilka sond) | brak / DS18B20 |

Na kanale DS18B20 może pracować kilka sond (unikalne adresy 1-Wire); DHT22 zawsze jeden na kanał i nie dzieli linii z DS18B20. DS18B20 to jeden typ sondy dla dwóch zastosowań: temperatura wody (Aqua) i temperatura powierzchni wygrzewania (Terra) — w wersji wodoodpornej reaguje w powietrzu z opóźnieniem kilku minut, a w promieniach lampy pokazuje temperaturę nagrzanej powierzchni, a nie powietrza. Dla miejsca wygrzewania osobne progi alarmowe (za gorąco = ryzyko poparzenia). Każdy czujnik ma w panelu własną nazwę (np. „woda”, „strefa ciepła”).

## Sensor MINI (2 × DHT22)
- Odczyt przez sprzętowy odbiornik RMT ESP32-C3 zamiast programowego odmierzania czasu — przerwania Wi-Fi na jednordzeniowym C3 nie psują odczytu.
- Retry, odstęp min. 2 s na czujnik, znacznik błędu / braku czujnika / złej sumy kontrolnej; nieaktualne dane nigdy nie są pokazywane jako bieżące.
- Panel: bieżące odczyty, wykres 24 h, progi alarmowe (za ciepło / za sucho) z ostrzeżeniem. Bez sterowania urządzeniami wykonawczymi — to rola Sensor PRO.

## Z WeatherLight
- **Bierzemy:** OTA z GitHuba, captive portal (poprawiony), ustawienia w NVS, szkielet panelu WWW z REST, wyliczanie słońca, wybór lokalizacji na mapie, `weather_api.h` (pogoda na żywo — opcja), `timer_mode`, tłumaczenia PL/EN, z silnika efektów — tylko chmury (i błyski burzy).
- **Nie bierzemy:** efekty pogodowe WeatherLight 1:1 (pogoda na żywo w TerraLight steruje scenami biotopu przez zabezpieczenia), efekty świąteczne, dashboard, status bar, tryb nocny z PIR.

## Uwagi
- RGB z WS2812/WS2811 nie zastępuje oświetlenia dziennego wymaganego gatunkowo, UVB ani lampy do uprawy roślin: cały system ma ok. 10 W, a pojedyncze diody RGB mają niewielką moc i wąskie pasma barw.

## Znane błędy do poprawy (z v1.0.0 / WeatherLight v6.3.0)
- [x] **Strefa czasowa:** _(naprawione w v2 M1: `core/clock`)_ `configTime(3600, 3600, …)` daje UTC+2 przez cały rok — zimą zegar spieszy się o 1 h. Dotyczy TerraLight v1.0.0 **i WeatherLight v6.3.0**. Poprawka: `configTzTime()` z regułą POSIX.
- [x] **Blokujący captive portal:** _(naprawione w v2 M1: `net/wifi_manager`)_ pętla `while (apMode)` zatrzymuje cykl dnia, LED gasną po 60 s.
- [x] _(naprawione w v2 M1)_ Captive portal używa stałej `NUMPIXELS` zamiast skonfigurowanej liczby LED.
- [x] _(v2: `core/pins.h` wg HARDWARE.md)_ Pinout v1 (LED na GPIO2 — pin startowy, przycisk na GPIO5) nieaktualny względem hardware v0.1.

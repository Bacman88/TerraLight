# TerraLight firmware v2.0 — architektura

Status: projekt architektury, wersja robocza.
Zakres funkcji: [FIRMWARE.md](FIRMWARE.md). Ten dokument opisuje, *jak* to zbudować.

---

## 1. Repozytorium i wydania
Projekt **w pełni open source** — hardware, dokumentacja i kod firmware w jednym publicznym repo `Bacman88/TerraLight` (licencja: [LICENSE](../LICENSE)).

- Kod firmware w katalogu `firmware/`, dokumentacja w `docs/`.
- **Releases** z gotowymi plikami `.bin` — źródło aktualizacji OTA (`releases/latest`) oraz do wgrania bez kompilacji.
- Wydanie: tag wersji → build → `.bin` dołączony do release'u (później automatycznie przez GitHub Actions).

## 2. Środowisko
- **PlatformIO** w VS Code / VSCodium (konfiguracja środowiska później, gdy będzie wolne okno od WMS).
- Platforma: **pioarduino** `55.03.312-1` = ESP32 Arduino core **3.3.12** (ESP-IDF 5.5). Oficjalna platforma `espressif32` w PlatformIO zostaje na core 2.x.
- Płyta: ESP32-C3 SuperMini, 4 MB flash, natywne USB (`ARDUINO_USB_CDC_ON_BOOT=1`).
- Partycje: `min_spiffs.csv` — dwa sloty aplikacji OTA po 1,9 MB + NVS + mały SPIFFS. Domyślny układ (1,25 MB) był zapełniony w 86 % już w M1 (Wi-Fi + HTTP + mDNS ≈ 1 MB). Historia czujników nie jest zapisywana do flash — patrz 7.
- Kompilacja kontrolna każdej wersji przed wydaniem: `arduino-cli` z tym samym core 3.3.12 (środowisko Claude'a nie ma dostępu do rejestru PlatformIO), docelowo także GitHub Actions z PlatformIO.

### Biblioteki
| Biblioteka | Do czego |
|---|---|
| Adafruit NeoPixel | LED (na C3 przez sprzętowy RMT) |
| ArduinoJson 7 | REST API, ustawienia, Open-Meteo |
| OneWire + DallasTemperature | DS18B20 |
| własny sterownik | DHT22 przez odbiornik RMT |
| WebServer + DNSServer (core) | panel i captive portal — jak w WeatherLight |
| HTTPClient + WiFiClientSecure (core) | OTA, Open-Meteo |

Kanały RMT ESP32-C3 (2 × TX, 2 × RX): LED → TX0, DHT A → RX0, DHT B → RX1.

## 3. Struktura katalogów
```
TerraLight/
├─ docs/                       HARDWARE, FIRMWARE, ARCHITECTURE…
└─ firmware/
   ├─ platformio.ini
   ├─ src/
   │  ├─ main.cpp                 setup() + pętla zadań
   │  ├─ core/
   │  │  ├─ pins.h                pinout (z HARDWARE.md, po zatwierdzeniu)
   │  │  ├─ version.h
   │  │  ├─ settings.*            struktura ustawień, NVS, wersja schematu, migracje, wartości domyślne Terra/Aqua
   │  │  ├─ clock.*               NTP, strefa POSIX, czas z telefonu, flaga „czas pewny / niepewny”
   │  │  ├─ astro.*               wschód/zachód, faza i jasność księżyca (lokalnie)
   │  │  └─ log.*
   │  ├─ net/
   │  │  ├─ wifi_manager.*        maszyna stanów STA/AP, nieblokująca
   │  │  ├─ captive_portal.*      DNS + strona konfiguracji Wi-Fi
   │  │  ├─ ota_github.*          sprawdzanie i aktualizacja z Releases
   │  │  └─ weather_live.*        Open-Meteo → zdarzenia pogodowe (opcja)
   │  ├─ light/
   │  │  ├─ led_output.*          bufor ramki, kolejność kolorów, gamma, limit jasności i prądu, show()
   │  │  ├─ module_profile.*      RING 8/12, ARC, LINE…: liczba, typ, kształt, mA/piksel
   │  │  ├─ geometry.*            pozycja 0…1 → piksele (pierścień/łuk/linia), start, kierunek, zakres; maska gęstości „co N-ta”
   │  │  ├─ day_context.*         harmonogram → faza, postęp, pozycja słońca, faza księżyca
   │  │  ├─ compositor.*          składanie warstw (patrz 5)
   │  │  ├─ scenes/               static, day_full, sun_walk, plants, jungle, aqua_caustics, night_moon, observation, service
   │  │  └─ climate/
   │  │     ├─ biotopes.*         presety: palety, sceny, prawdopodobieństwa zdarzeń
   │  │     ├─ weather_engine.*   generator zdarzeń (losowy z ziarnem daty / na żywo)
   │  │     └─ welfare.*          zabezpieczenia: noc/świt bez burz, min. jasność, limity
   │  ├─ sensors/
   │  │  ├─ dht_rmt.*             DHT22 przez RMT RX
   │  │  ├─ ds18b20.*             1-Wire, wyszukiwanie sond, adresy
   │  │  ├─ sensor_manager.*      kanały DATA1/DATA2, typ wg ustawień, statusy, alarmy
   │  │  └─ history.*             bufor pierścieniowy 24 h w RAM
   │  ├─ input/
   │  │  └─ button.*              BOOT/GPIO9: przytrzymanie 5 s → reset Wi-Fi
   │  └─ web/
   │     ├─ server.*              trasy HTTP
   │     ├─ api_*.cpp             handlery REST (status, config, override, sensors, system)
   │     └─ assets.h              wygenerowany: panel (HTML/JS/CSS) skompresowany gzip
   ├─ web/                        źródła panelu: index.html, app.js, style.css, i18n.js
   └─ tools/build_assets.py       web/ → gzip → src/web/assets.h (uruchamiany przez PlatformIO przed buildem)
```

## 4. Pętla główna i zadania
ESP32-C3 ma jeden rdzeń — wszystko musi być nieblokujące.

| Zadanie | Częstotliwość | Gdzie |
|---|---|---|
| Render LED (compositor → show) | 40 FPS | `loop()` |
| Serwer WWW + DNS (AP) | każdy obieg | `loop()` |
| Wi-Fi manager, przycisk | 50–100 ms | `loop()` |
| Day context, klimat | 1 s | `loop()` |
| Czujniki (naprzemiennie kanały) | 2–5 s | `loop()`, RMT sprzętowo |
| Historia czujników | 5 min | `loop()` |
| Zapytania sieciowe: NTP, OTA check, Open-Meteo | rzadko | **osobne zadanie FreeRTOS** — zapytania HTTPS trwają do kilku sekund i nie mogą zamrażać światła |

Zasada: żadnego `delay()` dłuższego niż kilka ms, żadnej pętli `while` czekającej na sieć (błąd z v1.0.0: blokujący captive portal).

## 5. Składanie obrazu — warstwy
Każda klatka powstaje w tej kolejności:

1. **DayContext** — z zegara i harmonogramu: faza (noc / świt / dzień / zmierzch), postęp fazy, pozycja słońca 0…1, jasność dnia 0…1, faza księżyca. Czas niepewny → wymuszona „bezpieczna noc”.
2. **Scena dzienna** (z profilu Terra/Aqua) renderuje bazowy obraz do bufora RGB float.
3. **Klimat** — aktywne zdarzenia (chmura, deszcz, burza, sjesta) jako modyfikatory: przyciemnienie fragmentów, zmiana barwy, błysk. Kaustyki Aqua jako nakładka.
4. **Noc** — w fazie nocnej scena nocna (księżyc / niebieska poświata / czerwień / wyłączone), płynne przenikanie przez zmierzch.
5. **Nadpisanie chwilowe** — obserwacja, serwis, demo; z czasem trwania i automatycznym powrotem.
6. **Łagodny start i przejścia** — przenikanie przy starcie urządzenia (ok. 1 min) i przy każdej zmianie trybu.
7. **Wyjście** — maska gęstości „co N-ta”, globalny limit jasności, gamma, **limiter prądu** (suma szacowanego poboru wg profilu → skalowanie całej klatki), kolejność kolorów, `show()`.

Sceny nie wiedzą nic o zasilaniu, kolejności kolorów ani gęstości — robi to wyjście. Dzięki temu nowa scena to jeden plik z funkcją `render()`.

```cpp
struct SceneContext { const DayContext& day; const Geometry& geo; const ClimateState& climate; uint32_t nowMs; };
class Scene { public: virtual void render(const SceneContext&, FrameBuffer&) = 0; };
```

## 6. Ustawienia
- Jedna struktura `Settings` z sekcjami: `module`, `control`, `day`, `night`, `climate`, `sensors`, `location`, `system`.
- Zapis w NVS jako blob z **numerem wersji schematu**; przy aktualizacji OTA migracja ze starszej wersji zamiast utraty ustawień.
- Wartości domyślne zależne od profilu (Aqua: dłuższy świt, niebieska noc, DS18B20 na DATA1).
- Zapis odroczony (np. 2 s po ostatniej zmianie) — ochrona flash przy przesuwaniu suwaków.
- Historia schematu: 1 = `2.0.0-dev.1`; 2 = dodane `system.setupDone` (migracja 1 → 2 ustawia `true`, żeby skonfigurowane urządzenie nie dostało kreatora po OTA).
- **Kopia zapasowa:** eksport / import ustawień jako plik JSON z panelu.

## 7. Czujniki
- Każdy kanał: typ (DHT22 / DS18B20 / brak), nazwa, progi alarmowe, status (`ok`, `brak`, `błąd CRC`, `nieaktualny`).
- DHT22 przez RMT RX + retry; DS18B20 przez OneWire z kontrolą CRC, wyszukiwanie sond przy starcie i na żądanie z panelu, nazwy przypisane do adresów sond.
- Historia 24 h w RAM: próbka co 5 min = 288 punktów na sondę (kilka KB). Po restarcie historia znika — świadomie, by nie zużywać flash.

## 8. REST API panelu
JSON, bez autoryzacji w sieci lokalnej (jak WeatherLight).

| Metoda | Ścieżka | Opis |
|---|---|---|
| GET | `/api/status` | czas i jego źródło, faza, pozycja słońca, księżyc, aktywna scena i zdarzenia klimatu, czujniki, Wi-Fi, wersja |
| GET / POST | `/api/config` | całość lub sekcja (`?section=day`) |
| POST | `/api/preview` | podgląd parametrów sceny na żywo bez zapisu (dobór kolorów) |
| POST | `/api/override` | `observation` / `service` / `demo` / `off`, czas w minutach |
| GET | `/api/sensors/history` | historia 24 h |
| POST | `/api/sensors/scan` | ponowne wyszukanie sond DS18B20 |
| POST | `/api/time` | czas z telefonu (epoch) i strefa |
| GET | `/api/biotopes` | lista presetów biotopów |
| GET / POST | `/api/wifi` | skan sieci, zapis (AP i STA) |
| POST | `/api/ota/check`, `/api/ota/update` | aktualizacja |
| GET / POST | `/api/backup` | eksport / import ustawień |
| POST | `/api/system/restart` | restart |

## 9. Panel WWW
- Jedna strona (vanilla JS, bez frameworka), osadzona w firmware jako gzip — działa w trybie AP i bez internetu.
- Kreator pierwszego uruchomienia — **tylko w trybie AP** (`192.168.4.1`), dopóki `system.setupDone == false`: **Wi-Fi** (z „Pomiń”) → **liczba LED + zasilacz** (budżet prądu) → Zakończ. Wszystko inne zostaje na wartościach domyślnych i zmienia się później w panelu; pod `terralight.local` kreatora nie ma. Podczas kreatora AP nie zamyka się po udanym teście sieci (`wifi::holdAp`), tylko ok. 30 s po „Zakończ”. _(Decyzja 2026-10-14, zastępuje wcześniejszy 5-krokowy kreator.)_
- Wygląd jak panel WeatherLight: ciemny granat `#1a1a2e` / karty `#16213e`, akcent cyjan `#00d4ff`.
- Zakładki: **Pulpit** (stan, czujniki, szybkie przyciski: obserwacja / serwis / demo), **Światło dzienne**, **Harmonogram**, **Noc**, **Biotop i klimat**, **Czujniki**, **Ustawienia** (moduł, Wi-Fi, lokalizacja, system, kopia, aktualizacja).
- Mapa lokalizacji (Leaflet + OSM z CDN) ładowana tylko na żądanie; zawsze dostępne pola współrzędnych i lista gotowych lokalizacji.
- Tłumaczenia PL/EN po stronie przeglądarki (`i18n.js`) — mniej tekstów w pamięci ESP.

## 10. Kolejność prac (kamienie milowe)
| # | Zakres | Test na sprzęcie |
|---|---|---|
| M1 | szkielet: pins, settings, clock (TZ!), wifi manager + nieblokujący AP, przycisk, led_output + profile + limiter, scena „stałe”, minimalny panel | świeci, łączy się, AP nie gasi światła, reset przyciskiem |
| M2 | day_context, astro, sceny dzień / wędrujące słońce, noc z księżycem, łagodny start, demo | pełna doba w trybie demo |
| M3 | czujniki: DHT RMT, DS18B20, kanały, historia, alarmy | odczyty przy włączonym Wi-Fi, długie przewody |
| M4 | klimat: biotopy, generator zdarzeń, zabezpieczenia, dżungla, burza, rośliny, sceny Aqua, kaustyki, sjesta | tydzień pracy, przegląd losowości w demo |
| M5 | nadpisania (obserwacja, serwis), pogoda na żywo, OTA, kopia ustawień, pełny panel PL/EN | aktualizacja OTA z release'u |
| — | **v2.0.0** | |

## 11. Otwarte kwestie
- Zatwierdzony pinout płytki SuperMini (HARDWARE.md: „do zatwierdzenia”).
- RTC na AUX1/AUX2 — tak / nie (wpływa na `clock`).
- Pierwszy build w PlatformIO u użytkownika — potwierdzenie platformy `55.03.312-1`.
- Paleta barw i parametry biotopów — dobór na prototypie.

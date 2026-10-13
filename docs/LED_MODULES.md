# TerraLight — moduły LED

Wspólne wyjście LED OUT: 5 V / DATA / GND. Podłączany jeden wybrany moduł. Złącze roboczo JST-SM 3-pin; zweryfikować polaryzację wiązki, bo gotowe przewody producentów mogą mieć różną kolejność pinów.

| Wariant | Liczba | Forma |
|---|---:|---|
| RING 8 | 8 | pierścień WS2812B |
| RING 12 | 12 | pierścień WS2812B |
| ARC | 15–20 | pojedyncze adresowalne piksele na giętkim przezroczystym przewodzie |
| LINE 60 | 30 | pasek ok. 60 cm obok głównego oświetlenia |

LED są adresowalne niezależnie; firmware ma konfigurowalną liczbę pikseli, model układu i limit poboru.

## Referencyjny łańcuch do prób i seryjnej produkcji
GFLAI WS2811 Pixel Fairy Lights: 5 V, rozstaw 5 cm, przezroczysty przewód preferowany, IP67/IP68 zależnie od wybranego wariantu.
- https://www.pixelfairylights.com/fairy-lights-pixel-led-ws2811/
- https://shop.gflai.com/products/5v5cm-color-changing-32-8-ft-200-led-christmas-addressable-fairy-string-lights

Do prób rozważane 200 diod / ok. 10 m; docelowo tani dostawca z Chin, potencjalnie wariant 2,5 cm. Weryfikować prąd (deklarowane 20 mA/LED nie jest uniwersalną właściwością WS2811), rzeczywiste IP, trwałość przewodów, cięcie i ponowne uszczelnianie.

LINE 30 na zwykłych WS2812B przy pełnej bieli nie mieści się komfortowo w budżecie zasilacza 5 V / 2 A. Dla dużych terrariów przewidujemy osobną wersję z mocniejszym zasilaczem.

## Efekty / wymagania dla firmware
Świt, zmierzch przed/po pracy lampy głównej, chmury, księżyc/fazy, przemieszczanie punktu świetlnego (wschód → zenit → zachód), harmonogram lokalny, opcjonalnie Open-Meteo. Nie zastępuje oświetlenia UVB ani podstawowego oświetlenia dziennego dla zwierząt i roślin.

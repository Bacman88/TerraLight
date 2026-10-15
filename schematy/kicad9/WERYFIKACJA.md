# TerraLight KiCad 9 — lista weryfikacyjna

Źródło: [docs/PCB.md](../../docs/PCB.md). **Schemat v0.2 jest szkicem niezweryfikowanym w KiCad 9**, nie nadaje się bez kontroli do produkcji.

1. Uruchomić ERC i sprawdzić wszystkie połączenia i footprinty.
2. ESP32-C3-MINI-1-N4: zastosować **pełny symbol producenta**, w tym wszystkie piny GND/NC oraz pady pod modułem; numerów GPIO nie mylić z numerami padów.
3. Sprawdzić numerację USB-C 16-pin, JST-VH 3-pin (LED), JST-GH 6-pin (SENSOR) względem rzeczywistych footprintów.
4. Potwierdzić układy: 74AHCT1G125, ESD USBLC6-2SC6, LDO 3,3 V, P-MOSFET, 2N7002, diody TVS i Schottky.
5. Zweryfikować klucz LED przy 2,5 A, soft-start, PTC, termikę, udary i budżet USB-C 3 A z uwzględnieniem ESP.
6. Rozważyć zasilanie zwrotne wejścia DATA przy wyłączonym LED_5V.
7. Sprawdzić GPIO2/8/9 (bootstrapping), EN i przyciski.
8. Zaprojektować strefę anteny bez miedzi według dokumentacji Espressif.
9. Dobrać rezystory i kondensatory filtrujące ADC oraz pomiar CC.
10. Po pełnym ERC przeprowadzić kontrolę PCB DRC, obciążalności ścieżek i przygotowania BOM/CPL pod JLCPCB.

**Nie zamawiać PCB na podstawie szkicu.**

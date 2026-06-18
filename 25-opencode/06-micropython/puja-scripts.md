# Pujar scripts MicroPython (ESP8266) — prompt per a OpenCode

Copia i enganxa això a OpenCode:

```
Tinc una NodeMCU V2 (ESP8266) amb MicroPython ja flashejat. Vull pujar-hi scripts.

Primer llegeix el boot.py i els exercicis de la carpeta 35-micropython-esp8266/ del repositori.

Després:

1. Puja boot.py al dispositiu (el trick de GPIO0 per evitar main.py)
2. Puja l'exercici que et digui (00, 01, 02 o 03) com a main.py
3. Fes reset de la placa

Comandes a usar:
- mpremote connect <PORT> fs cp boot.py :boot.py
- mpremote connect <PORT> fs cp exerciciXX.py :main.py
- mpremote connect <PORT> reset

Detecta el port automàticament. Si la placa no respon, prova:
1. Connecta GPIO0 a GND
2. Prem RST
3. Intenta de nou
```

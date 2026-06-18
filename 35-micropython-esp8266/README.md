# 🐍 MicroPython per ESP8266 (NodeMCU)

Exercicis progressius per aprendre a programar plaques **NodeMCU (ESP8266)** amb **MicroPython**.

## 📋 Exercicis

| # | Fitxer | Què s'aprèn | Material |
|---|--------|-------------|----------|
| 00 | `exercici00.py` | Blink simple amb `time.sleep_ms` | LED + resistència 220 Ω |
| 01 | `exercici01.py` | 4 LEDs asíncrons amb `uasyncio`, intervals primers, monitor de memòria | 4 LEDs + 4 × 220 Ω |
| 02 | `exercici02.py` | 01 + NeoPixel cicle RGB | + NeoPixel (GPIO15) |
| 03 | `exercici03.py` | 02 + sensor LM75B per I2C | + LM75B (SCL=GPIO5, SDA=GPIO4) |

## 🚀 Primers passos

1. Segueix la **[guia de flash](flashejar_nodemcu_micropython.md)** per instal·lar MicroPython a la NodeMCU
2. Puja `boot.py` per tenir el trick de recuperació (GPIO0 a GND → no executa `main.py`)
3. Copia l'exercici que vulguis com a `main.py` al dispositiu:
   ```bash
   mpremote connect /dev/ttyUSB0 fs cp exercici00.py :main.py
   mpremote connect /dev/ttyUSB0 reset
   ```

## 🔧 GPIOs de l'ESP8266 (NodeMCU)

| GPIO | Funció | Notes |
|------|--------|-------|
| 16 | LED integrat (alguns models) | Sortida lliure |
| 0 | Flash button / BOOT | Entrada amb PULL_UP; GND = mode flash |
| 2 | LED blau integrat | Sortida; LOW = encès |
| 13 | Sortida lliure | |
| 14 | Sortida lliure | |
| 15 | NeoPixel | |
| 5 | I2C SCL | |
| 4 | I2C SDA | |
| 3 (RX) | **No usar** com a sortida | Interfereix amb UART |

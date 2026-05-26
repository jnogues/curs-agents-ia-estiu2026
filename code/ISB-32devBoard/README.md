# 🧩 ISB-32devBoard

> Placa de desenvolupament educativa basada en **ESP32-S3 DevKit C1 N16R8**

---

## 📋 Especificacions del xip

| Paràmetre        | Valor                        |
| ---------------- | ---------------------------- |
| Xip              | ESP32-S3                     |
| Flash            | 16 MB (N16)                  |
| PSRAM            | 8 MB (R8)                    |
| CPU              | Dual-core Xtensa LX7 @ 240 MHz |
| WiFi             | 802.11 b/g/n (2.4 GHz)       |
| Bluetooth        | BLE 5.0                      |
| GPIOs disponibles | 24 (dels 45 totals)         |
| ADC              | 2 × SAR ADC (12 bits)        |
| DAC              | 2 × 8 bits                   |

---

## 🔌 Assignació de GPIOs

### Entrades analògiques i digitals

| GPIO | Element              | Tipus        | Notes                   |
| ---- | -------------------- | ------------ | ----------------------- |
| 1    | Potenciòmetre (RVAR) | ADC          |                         |
| 2    | LDR (llum)           | ADC          |                         |
| 4    | Sensor Hall          | ADC          |                         |
| 5    | Touch                | GPIO / Touch | TOUCH5 / RTC            |

### Sortides de potència

| GPIO | Element                   | Tipus         | Notes                          |
| ---- | ------------------------- | ------------- | ------------------------------ |
| 6    | Triac (bombeta 230V)      | Sortida       | Via optoacoblador              |
| 17   | PWM                       | Sortida       | Resistència calefactora (5 kHz) |
| 18   | Relé                      | Sortida       |                                |

### LEDS

| GPIO | Element  | Notes            |
| ---- | -------- | ---------------- |
| 7    | LED 1    |                  |
| 15   | LED 2    |                  |
| 21   | LED 3    |                  |
| 47   | LED 4    |                  |
| 48   | RGB LED  | NeoPixel WS2812  |

### Polsadors

| GPIO | Element    | Notes |
| ---- | ---------- | ----- |
| 39   | Polsador 1 |       |
| 40   | Polsador 2 |       |
| 41   | Polsador 3 |       |
| 42   | Polsador 4 |       |

### Àudio

| GPIO | Element | Notes                     |
| ---- | ------- | ------------------------- |
| 38   | Buzzer  | Freqüència + durada       |

---

## 🔗 Buses de comunicació

| Bus    | GPIOs                                          | Dispositiu                     |
| ------ | ---------------------------------------------- | ------------------------------ |
| **I2C**    | SDA = **8**, SCL = **9**                       | LM75A (sensor temperatura)     |
| **1-Wire** | DQ = **16**                                    | DS18B20 (sensor temperatura)   |
| **SPI**    | MOSI = **11**, SCLK = **12**, CS = **10**, DC = **13**, RST = **14** | GC9A01A (display circular 240×240) |

### Detall dels buses

```
┌────────────────────────────────────────────────┐
│                   ESP32-S3                      │
│                                                 │
│  GPIO 8  ─── SDA ────────┬────────────────────  │
│  GPIO 9  ─── SCL ────────┘  LM75A (I²C)        │
│                                                 │
│  GPIO 16 ─── DQ ──────────  DS18B20 (1-Wire)    │
│                                                 │
│  GPIO 11 ─── MOSI ────────┐                     │
│  GPIO 12 ─── SCLK ────────┤  GC9A01A (SPI)      │
│  GPIO 10 ─── CS  ─────────┤  Display circular   │
│  GPIO 13 ─── DC  ─────────┤  240×240 px         │
│  GPIO 14 ─── RST ─────────┘                     │
└────────────────────────────────────────────────┘
```

---

## 🧠 Resum de funcionalitats per bloc

| Bloc               | GPIOs usats | Descripció                                     |
| ------------------ | ----------- | ---------------------------------------------- |
| 🔆 **Sensors**     | 1, 2, 4     | Potenciòmetre, LDR, Hall (tots ADC)            |
| 👆 **Touch**       | 5           | Entrada capacitiva                             |
| 💡 **Il·luminació** | 6, 7, 15, 21, 47, 48 | Triac (230V), LEDs digitals, NeoPixel RGB |
| 🔥 **Calefactor**  | 17          | PWM 5 kHz per resistència calefactora          |
| 🔄 **Relé**        | 18          | Commutació ON/OFF                              |
| 🔊 **So**          | 38          | Buzzer programable (freqüència + durada)       |
| 🎮 **Polsadors**   | 39, 40, 41, 42 | 4 botons d'entrada digital                  |
| 🌡️ **Temperatura** | 8, 9, 16    | LM75A (I²C) + DS18B20 (1-Wire)                |
| 🖥️ **Display**     | 10, 11, 12, 13, 14 | GC9A01A circular 240×240 (SPI)        |

---

## ⚠️ Notes importants

1. **GPIO 6 (Triac)**: Controla una bombeta de 230V AC **via optoacoblador**. Mai connectar directament!
2. **GPIO 17 (PWM)**: La resistència calefactora funciona a **5 kHz** per evitar soroll audible.
3. **GPIO 48 (NeoPixel)**: El WS2812 requereix una llibreria NeoPixel i un condensador de desacoblament proper al LED.
4. **ADC**: GPIOs 1, 2 i 4 són ADC de 12 bits (0-4095). Tensió màxima 3.3 V.
5. **Display GC9A01A**: És un display **circular** de 240×240 píxels, requereix llibreria gràfica (TFT_eSPI o similar).
6. **Pull-ups I²C**: Els pins SDA/SCL del LM75A necessiten resistències pull-up externes (4.7 kΩ típic).

### 🚫 GPIOs **NO** disponibles

Aquests GPIOs de l'ESP32-S3 **no s'han d'utilitzar** a la ISB-32devBoard:

| GPIO | Motiu                       |
| ---- | --------------------------- |
| 0    | Strapping pin (boot mode)   |
| 3    | Strapping pin / JTAG        |
| 19   | USB D- (natiu)              |
| 20   | USB D+ (natiu)              |
| 35   | Reservat intern             |
| 36   | Reservat intern             |
| 37   | Reservat intern             |
| 43   | UART TX (sèrie per defecte) |
| 44   | UART RX (sèrie per defecte) |
| 46   | Strapping pin               |

> ⚡ **No connectis res a aquests GPIOs** — alguns decideixen el mode d'arrencada del xip i poden impedir que l'ESP32-S3 funcioni correctament.

---

## 📁 Estructura de la carpeta

```
ISB-32devBoard/
├── README.md        ← Aquest fitxer
├── datasheets/      ← Fitxes tècniques dels components
└── schematics/      ← Esquemes elèctrics (futur)
```

---

> **I2SB · Institut Indústria Sostenible de Barcelona** · Curs d'Instal·lació d'Agents d'IA · Estiu 2026

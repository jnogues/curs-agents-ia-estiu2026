# 🧩 ISB-32devBoard

> Placa de desenvolupament educativa basada en **ESP32-S3 DevKit C1 N16R8**
>
> **Institut Sabadell** · Curs d'Instal·lació d'Agents d'IA · Estiu 2026

---

## 🔌 Assignació de GPIOs

### Entrades analògiques

| GPIO | Element              | Tipus | Notes                        |
| ---- | -------------------- | ----- | ---------------------------- |
| 1    | Potenciòmetre (RVAR) | ADC   |                              |
| 2    | LDR (llum)           | ADC   |                              |
| 4    | Sensor Hall          | ADC   | HAL49ESO                     |
| 5    | Touch capacitivo     | ADC   | TOUCH5 / RTC                 |

### Sortides de potència

| GPIO | Element                   | Tipus   | Notes                          |
| ---- | ------------------------- | ------- | ------------------------------ |
| 6    | Triac (bombeta 230V)      | Sortida | MOC3063 + BT136, via opto      |
| 17   | PWM                       | Sortida | Resistència calefactora (5 kHz) |
| 18   | Relé                      | Sortida | PC817C + AO3400                |

### LEDs

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
| 38   | Buzzer  | 2 kHz, freqüència + durada |

---

## 🔗 Buses de comunicació

| Bus    | GPIOs                                          | Dispositiu                   | Model          |
| ------ | ---------------------------------------------- | ---------------------------- | -------------- |
| **I²C**    | SDA = **8**, SCL = **9**                       | Sensor temperatura           | TMP75AIDR      |
| **1-Wire** | DQ = **16**                                    | Sensor temperatura           | DS18B20        |
| **SPI**    | MOSI = **11**, SCLK = **12**, CS = **10**, DC = **13**, RST = **14** | Display circular 1.28" 240×240 | GC9A01     |

```
               ┌─────────────────────────────────┐
               │           ESP32-S3               │
               │                                  │
GPIO 8  ───────┤ SDA (I²C)                        │
GPIO 9  ───────┤ SCL (I²C)      ─── TMP75AIDR     │
GPIO 16 ───────┤ DQ (1-Wire)    ─── DS18B20       │
GPIO 11 ───────┤ MOSI                             │
GPIO 12 ───────┤ SCLK            ─── GC9A01       │
GPIO 10 ───────┤ CS              (Display SPI)    │
GPIO 13 ───────┤ DC                               │
GPIO 14 ───────┤ RST                              │
               └─────────────────────────────────┘
```

---

## 🚫 GPIOs **NO** disponibles (no connectar-hi res!)

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

---

## 📁 Contingut de la carpeta

```
devboard/
├── README.md                       ← GPIO mapping i descripció
├── schematics/
│   └── ISB-32devBoard_Esquematic.pdf    ← Esquemàtic (EasyEDA)
└── pcb/
    ├── ISB-32devBoard_PCB_front.webp    ← PCB cara frontal
    └── ISB-32devBoard_PCB_back.webp     ← PCB cara posterior
```

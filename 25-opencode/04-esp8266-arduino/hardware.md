# Hardware NodeMCU (ESP8266) — curs I2SB

| Component | GPIO / Pin | Notes |
|-----------|-----------|-------|
| 🔴 LED integrat | **GPIO16** | LOW = encès, HIGH = apagat |
| 🟡 LED 1 | **GPIO0** | Sortida |
| 🟢 LED 2 | **GPIO2** | Sortida |
| 🔵 LED 3 | **GPIO13** | Sortida |
| 🌈 NeoPixel | **GPIO15** | Un sol LED RGB |
| 🌡️ LM75B SCL | **GPIO5** | I2C, adreça 0x48 |
| 🌡️ LM75B SDA | **GPIO4** | I2C |
| 🔄 Potenciòmetre | **A0** | ADC, 0-3.3V → 0-1024 |
| 🔘 Polsador 1 | **GPIO12** | Pull-down extern |
| 🔘 Polsador 2 | **GPIO14** | Pull-down extern |

> ⚠️ **GPIO3 (RX)**: No usar com a sortida — interfereix amb la comunicació sèrie.

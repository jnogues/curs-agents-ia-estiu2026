# 02 — Sensor de temperatura extern DS18B20

Evolució de l'Exercici-01: afegeix un **sensor de temperatura extern DS18B20**
(petita càpsula negra de 3 pins, molt comuna) connectat al GPIO16.

Ara l'ESP32 publica **dues temperatures**: la interna del xip i l'externa del DS18B20.

---

## 🆕 Novetats respecte a l'Exercici-01

| Concepte | Exercici-01 | Exercici-02 |
|----------|-------------|-------------|
| Tasques FreeRTOS | 3 | **4** |
| Sensors de temperatura | Intern (xip) | **Intern + DS18B20 extern** |
| Llibreries noves | — | **OneWire** + **DallasTemperature** |
| GPIO extra | — | **GPIO 16** (DS18B20 data) |
| Tòpic MQTT nou | — | `/esp32s3/…/**temperaturaDS**` |
| Lectura asíncrona | — | ✅ `setWaitForConversion(false)` |

---

## 🛒 Material extra

- 1 × Sensor **DS18B20** (càpsula TO-92 o sonda metàl·lica)
- 1 × Resistència **4,7 kΩ** (pull-up entre VCC i data)
- Cables dupont

### Connexions

```
DS18B20        ESP32-S3
┌─────┐
│ VCC │──── 3,3 V
│ DAT │──── GPIO 16
│ GND │──── GND
└─────┘
       │
      ┌┤
     4,7 kΩ
      ┌┤
       │
     3,3 V
```

> ⚠️ La resistència pull-up de 4,7 kΩ és **obligatòria** entre VCC i DATA,
> sense ella el DS18B20 no funcionarà.

---

## 🧰 Llibreries noves

Instal·lar des de l'**Library Manager** (`Eines → Gestiona llibreries…`):

| Llibreria | Autor |
|-----------|-------|
| **OneWire** | Jim Studt |
| **DallasTemperature** | Miles Burton |

La resta (MycilaMQTT, Adafruit NeoPixel) són les mateixes que a l'Exercici-01.

---

## 🚀 Configuració

1. Obre `Exercici-02.ino` i canvia:
   ```cpp
   #define ALUMNE_ID       "alumneXX"     // posa el teu número
   ```
   (i el WiFi si cal)

2. Connecta el DS18B20 a GPIO 16 segons l'esquema anterior

3. Carrega a la placa

4. Obre el **Monitor Sèrie** (115200 baud). Hauries de veure:
   ```
   === ESP32-S3 Template ===
   [WiFi] Connectat a 'SSID_ALUMNE'  IP: 192.168.x.x
   [MQTT] Connectat al broker
   [TEMP INT] 32.15 °C
   [TEMP DS] 25.80 °C
   ```

---

## 📡 Tòpics MQTT

| Tòpic | Freqüència | Contingut |
|-------|-----------|-----------|
| `/esp32s3/<alumne>/estat` | cada 5 s | `{"ip":"...","rssi":-XX,"uptime":NN}` |
| `/esp32s3/<alumne>/temperaturaINT` | cada 10 s | `"32.15"` (ºC interna del xip) |
| `/esp32s3/<alumne>/temperaturaDS` | cada 10 s | `"25.80"` (ºC externa DS18B20) |
| `/esp32s3/<alumne>/ordre` | — | Ordres rebudes (pendent d'implementar) |

Per veure tot en directe:
```bash
mosquitto_sub -h 46.224.116.35 -t "/esp32s3/#" -v
```

---

## ⚙️ Arquitectura (4 tasques)

```
Core 1
┌─────────────────────────────────────────┐
│  neopixelTask   (prio 3 · 2 KB stack)   │ ← LED RGB
│  tempInternaTask (prio 4 · 4 KB)        │ ← Sensor intern del xip
│  tempDS18B20Task (prio 4 · 4 KB)        │ ← Sensor extern DS18B20 ⭐
│  mainTask       (prio 5 · 8 KB)         │ ← WiFi + MQTT
└─────────────────────────────────────────┘
```

La tasca `tempDS18B20Task` és la novetat: llegeix el DS18B20 de forma **asíncrona**
(`setWaitForConversion(false)`) — llança la conversió i espera el temps necessari
amb `vTaskDelay` en lloc de bloquejar-se, deixant espai per a les altres tasques.

---

## 🐛 Troubleshooting

| Símptoma | Causa | Solució |
|----------|-------|---------|
| `[TEMP DS] Sensor no detectat` | Cablejat incorrecte o falta pull-up | Revisa GPIO 16 i la resistència 4,7 kΩ |
| Temperatura DS sempre a 85 °C | Sensor en estat de reset | Revisa alimentació 3,3 V |
| `-127 °C` | Sensor no respon | Comprova connexions i OneWire |
| La temperatura no canvia | Toca el sensor amb els dits per escalfar-lo | Ha de respondre en < 1 s |

---

## 📁 Fitxers

| Fitxer | Descripció |
|--------|------------|
| `Exercici-02.ino` | Codi font amb suport DS18B20 |
| `README.md` | Aquest document |

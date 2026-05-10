# 01 — Codi Arduino · ESP32-S3

Template per a l'ESP32-S3 que connecta per WiFi i MQTT, amb visualització d'estat
via Neopixel integrat i lectura del sensor de temperatura intern.

---

## 🎯 Objectius de l'exercici

1. Programar un ESP32-S3 des de l'Arduino IDE
2. Entendre el model de **3 tasques FreeRTOS** concurrents
3. Connectar-se a una xarxa WiFi (amb fallback automàtic)
4. Publicar i subscriure's a tòpics **MQTT**
5. Interpretar l'estat del dispositiu amb el **LED RGB** integrat

---

## 🧰 Requisits

### Hardware

- Placa **ESP32-S3** (qualsevol variant amb Neopixel integrat, p. ex. ESP32-S3-DevKitC-1)
- Cable USB-C per a programació

### Programari (Arduino IDE)

Instal·lar des de l'**Library Manager** (`Eines → Gestiona llibreries…`):

| Llibreria | Autor | Versió testada |
|-----------|-------|----------------|
| **MycilaMQTT** | mathieucarbou | ≥ 5.0 |
| **Adafruit NeoPixel** | Adafruit | ≥ 1.12 |

**Board:** `ESP32S3 Dev Module` (o la teva variant d'ESP32-S3)

> 💡 **Consell:** Si no apareix, ves a `Eines → Placa → Gestor de plaques…`
> i instal·la `esp32` d'Espressif Systems.

---

## 🚀 Posada en marxa

### 1. Configuració pròpia de cada alumne

Obre `Exercici-01.ino` i canvia **el teu identificador**:

```cpp
#define ALUMNE_ID       "alumne01"     // <-- posa el teu número d'alumne
```

Si et connectes a una xarxa WiFi diferent del broker, canvia també:

```cpp
#define WIFI_SSID_1  "SSID_ALUMNE"
#define WIFI_PASS_1  "PASSWORD_ALUMNE"
```

### 2. Carregar a la placa

1. Connecta l'ESP32-S3 per USB
2. Selecciona `ESP32S3 Dev Module` i el port correcte
3. Prem **⇧ Carrega** (o `Ctrl+U`)

### 3. Verificar

Obre el **Monitor Sèrie** (`Ctrl+Mayúsc+M`, 115200 baud). Hauries de veure:

```
=== ESP32-S3 Template ===
[WiFi] Connectant...
[WiFi] Connectat a 'SSID_ALUMNE'  IP: 192.168.x.x
[MQTT] Connectant al broker...
[MQTT] Connectat al broker
```

El LED integrat ha de fer un **batec verd** (pols cada segon) — senyal de _"tot OK"_.

---

## 📡 Tòpics MQTT

| Tòpic | Direcció | Contingut |
|-------|----------|-----------|
| `/esp32s3/<alumne>/estat` | ESP32 → Broker | `{"ip":"...","rssi":-XX,"uptime":NN}` cada 5 s |
| `/esp32s3/<alumne>/temperaturaINT` | ESP32 → Broker | `"28.34"` (ºC intern) cada 10 s |
| `/esp32s3/<alumne>/ordre` | Broker → ESP32 | Ordres rebudes (pendent d'implementar) |

Pots subscriure-t'hi amb Node-RED, `mosquitto_sub` o un client MQTT qualsevol:

```bash
mosquitto_sub -h 46.224.116.35 -t "/esp32s3/#" -v
```

---

## 🎨 Llegenda del LED Neopixel

| Estat | Color | Patró | Significat |
|-------|-------|-------|------------|
| `ESTAT_INIT` | ⚪ Blanc | Fix | Inici del sistema |
| `ESTAT_WIFI_CONNECTANT` | 🔵 Blau | Parpelleig 1 Hz | Connectant al WiFi |
| `ESTAT_WIFI_OK_MQTT_CONN` | 🟡 Groc | Parpelleig 4 Hz | WiFi OK, connectant MQTT |
| `ESTAT_TOT_OK` | 🟢 Verd | Batec 1 Hz (pols curt) | **Tot correcte** ✅ |
| `ESTAT_WIFI_FALLAT` | 🔴 Vermell | Parpelleig 4 Hz | WiFi fallat (reservat) |
| `ESTAT_WIFI_OK_MQTT_PERDUT` | 🟠 Taronja | Parpelleig 1 Hz | WiFi OK, MQTT perdut |

---

## ⚙️ Arquitectura interna

Tres tasques FreeRTOS independents al **Core 1**:

```
┌─────────────────────────────────────┐
│  loop()       ← suspès (no gasta CPU) │
├─────────────────────────────────────┤
│  mainTask     (prio 5 · 8 KB stack) │ ← WiFi + MQTT + publicació estat
│  tempInternaTask (prio 4 · 4 KB)    │ ← Sensor temperatura intern
│  neopixelTask  (prio 3 · 2 KB)      │ ← LED RGB (animacions)
└─────────────────────────────────────┘
         Core 1
```

Cada tasca comparteix la variable `estatActual` (volàtil) per coordinar-se.

---

## 🐛 Troubleshooting

| Símptoma | Causa probable | Solució |
|----------|----------------|---------|
| LED vermell parpellejant | No troba WiFi | Revisa SSID/PASS |
| LED groc sense passar a verd | Broker MQTT inaccesible | Comprova que el broker estigui en marxa |
| El Monitor Sèrie no mostra res | Velocitat de bauds incorrecta | Posa 115200 baud |
| Error a la compilació | Falta llibreria | Instal·la MycilaMQTT i Adafruit NeoPixel |
| L'IDE no reconeix la placa | Driver USB | Prova "ESP32S3 Dev Module", revisa el port |

---

## 📁 Fitxers

| Fitxer | Descripció |
|--------|------------|
| `Exercici-01.ino` | Codi font complet del template |
| `README.md` | Aquest document |

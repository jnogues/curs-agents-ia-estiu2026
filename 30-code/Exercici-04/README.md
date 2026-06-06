# 04 — Telemetria unificada

Evolució de l'Exercici-03: en lloc de publicar les temperatures en tòpics
MQTT separats, ara s'**incorporen al JSON d'estat** juntament amb la xarxa WiFi
connectada (`ssid`).

Resultat: **menys tòpics, menys tràfic, missatge autosuficient**.

---

## 🆕 Novetats respecte a l'Exercici-03

| Concepte | Exercici-03 | Exercici-04 |
|----------|-------------|-------------|
| Tòpics de temperatura | `/temperaturaINT` + `/temperaturaDS` | ❌ Eliminats |
| JSON d'estat | `{"ip":…,"rssi":…,"uptime":…,"leds":[…]}` | + `ssid`, `tempInt`, `tempDS` |
| Nombre de tòpics actius | 5 | **3** |
| Hardware nou | — | Cap |
| Llibreries noves | — | Cap |

### JSON d'estat resultant (cada 5 s)

```json
{
  "ssid":    "NomXarxa",
  "ip":      "192.168.1.5",
  "rssi":    -62,
  "uptime":  124,
  "tempInt": 42.50,
  "tempDS":  23.75,
  "leds":    [1, 0, 0, 1]
}
```

> 🧠 **Per què és millor?** Un sol tòpic conté tota la informació del dispositiu.
> Si Node-RED, Home Assistant o qualsevol client es subscriu a `/estat`, ja ho té tot
> sense haver d'agregar múltiples tòpics.

---

## 🚀 Configuració

Obre `Exercici-04.ino` i canvia el teu identificador (i el WiFi si cal):

```cpp
#define ALUMNE_ID    "alumneXX"     // <-- posa el teu número d'alumne

#define WIFI_SSID_1  "SSID_ALUMNE"
#define WIFI_PASS_1  "PASSWORD_ALUMNE"
```

Carrega a la placa. El Monitor Sèrie (115200 baud) ha de mostrar:

```
=== ESP32-S3 Template v4 ===
[WiFi] Connectat a 'NomXarxa'  IP: 192.168.x.x
[MQTT] Connectat al broker
[TEMP INT] 42.50 °C
[TEMP DS] 23.75 °C
[MQTT TX] {"ssid":"NomXarxa","ip":"192.168.x.x","rssi":-62,"uptime":5,"tempInt":42.50,"tempDS":23.75,"leds":[0,0,0,0]}
```

---

## 📡 Tòpics MQTT

| Tòpic | Direcció | Retained | Contingut |
|-------|----------|----------|-----------|
| `/esp32s3/<id>/estat` | ESP32 → Broker | No | JSON complet cada 5 s (veure exemple amunt) |
| `/esp32s3/<id>/stateLEDS` | ESP32 → Broker | **Sí** ✅ | `{"leds":[1,0,0,1]}` s'actualitza amb cada comanda |
| `/esp32s3/<id>/ordre` | Broker → ESP32 | No | Comandes `ledX ON/OFF/TOG` |

### Exemples

```bash
# Subscriure's a tot (lectura en directe)
mosquitto_sub -h 46.224.116.35 -t "/esp32s3/alumneXX/#" -v

# Encendre LED1
mosquitto_pub -h 46.224.116.35 -t "/esp32s3/alumneXX/ordre" -m "led1 ON"

# Llegir l'últim estat dels LEDs (retained)
mosquitto_sub -h 46.224.116.35 -t "/esp32s3/alumneXX/stateLEDS" -C 1
```

---

## ⚙️ Arquitectura (4 tasques)

```
Core 1
├── neopixelTask    (prio 3 · 2 KB)  ← LED RGB + flash lila
├── tempInternaTask (prio 4 · 4 KB)  ← Llegeix sensor intern → desa a tempInt
├── tempDS18B20Task (prio 4 · 4 KB)  ← Llegeix DS18B20      → desa a tempDS
└── mainTask        (prio 5 · 8 KB)  ← WiFi, MQTT, LEDs, publica estat
```

El canvi clau respecte a l'Exercici-03 és **qui publica les temperatures**:

| | Exercici-03 | Exercici-04 |
|-|-------------|-------------|
| `tempInternaTask` | Llegeix + publica a MQTT | Llegeix + desa a `volatile float tempInt` |
| `tempDS18B20Task` | Llegeix + publica a MQTT | Llegeix + desa a `volatile float tempDS` |
| `mainTask` | Publica `estat` sense temperatures | Publica `estat` amb `tempInt` i `tempDS` |

---

## 🐛 Troubleshooting

| Símptoma | Causa | Solució |
|----------|-------|---------|
| `tempInt` o `tempDS` a `0.00` | Primera lectura no ha arribat encara | Normal els primers 10 s d'arrencada |
| `[TEMP DS] Sensor no detectat` | DS18B20 desconnectat o sense pull-up | Revisa GPIO 16 i la resistència 4,7 kΩ |
| El JSON d'estat no apareix | Sense connexió MQTT | Comprova el LED (ha de ser verd) |
| No reb comandes de LEDs | `ALUMNE_ID` incorrecte | Ha de coincidir amb el que envies al `mosquitto_pub` |

---

## 📁 Fitxers

| Fitxer | Descripció |
|--------|------------|
| `Exercici-04.ino` | Codi font amb telemetria unificada |
| `README.md` | Aquest document |

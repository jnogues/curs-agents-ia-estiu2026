# 03 — Control de LEDs per MQTT

Evolució de l'Exercici-02: afegeix **4 LEDs externs** que es controlen
enviant comandes **MQTT** des de qualsevol client (terminal, Node-RED, altre dispositiu).

L'ESP32 ara **escolta** ordres i pot actuar sobre el món físic! 🎛️

---

## 🆕 Novetats respecte a l'Exercici-02

| Concepte | Exercici-02 | Exercici-03 |
|----------|-------------|-------------|
| LEDs externs | — | **4 LEDs** (GPIO 7, 15, 47, 21) |
| Comandes MQTT | — | `led1 ON`, `led1 OFF`, `led1 TOG` |
| Estat retained | — | ✅ `stateLEDS` amb *retained* |
| Flash Neopixel | — | 💜 Flash lila en rebre comanda |
| Estat al JSON | — | `{"ip":…, "leds":[1,0,0,1]}` |

---

## 🛒 Material extra

- 4 × LED (qualsevol color)
- 4 × Resistència **220 Ω** (o 330 Ω)
- Cables dupont

### Connexions

```
GPIO  7  ───┤220 Ω├─── → LED1 (ànode llarg → GPIO)
GPIO 15  ───┤220 Ω├─── → LED2
GPIO 47  ───┤220 Ω├─── → LED3
GPIO 21  ───┤220 Ω├─── → LED4

Càtode curt de cada LED → GND
```

> 💡 Els LEDs s'activen amb **HIGH** (lògica positiva). El LED s'encén
> quan el GPIO està a 3,3 V.

---

## 📡 Comandes MQTT

Envia al tòpic `/esp32s3/<alumne>/ordre`:

| Comanda | Efecte |
|---------|--------|
| `led1 ON` | Encén el LED1 |
| `led1 OFF` | Apaga el LED1 |
| `led1 TOG` | Inverteix l'estat del LED1 |
| `led2 ON` / `led2 OFF` / `led2 TOG` | Idem per LED2 |
| `led3 …` / `led4 …` | Idem per LED3 i LED4 |

### Exemples:

```bash
# Encendre LED1
mosquitto_pub -h 46.224.116.35 -t "/esp32s3/alumneXX/ordre" -m "led1 ON"

# Invertir LED3
mosquitto_pub -h 46.224.116.35 -t "/esp32s3/alumneXX/ordre" -m "led3 TOG"

# Apagar LED2
mosquitto_pub -h 46.224.116.35 -t "/esp32s3/alumneXX/ordre" -m "led2 OFF"

# Veure-ho tot en directe
mosquitto_sub -h 46.224.116.35 -t "/esp32s3/alumneXX/#" -v
```

---

## 📡 Tòpics MQTT complets

| Tòpic | Direcció | Retained | Contingut |
|-------|----------|----------|-----------|
| `/esp32s3/<id>/estat` | ESP32 → Broker | No | `{"ip":"…","rssi":-XX,"uptime":NN,"leds":[1,0,0,1]}` cada 5 s |
| `/esp32s3/<id>/stateLEDS` | ESP32 → Broker | **Sí** ✅ | `{"leds":[1,0,0,1]}` s'actualitza amb cada comanda |
| `/esp32s3/<id>/ordre` | Broker → ESP32 | No | Comandes `ledX ON/OFF/TOG` |
| `/esp32s3/<id>/temperaturaINT` | ESP32 → Broker | No | Temperatura interna del xip cada 10 s |
| `/esp32s3/<id>/temperaturaDS` | ESP32 → Broker | No | Temperatura DS18B20 cada 10 s |

> 🔄 El tòpic `stateLEDS` es publica amb **retained=true**, així que
> si l'ESP32 es reinicia, recupera l'últim estat dels LEDs automàticament.

---

## 🎨 Comportament del Neopixel

A més dels estats habituals (verd = tot OK, blau = connectant…),
l'Exercici-03 afegeix un **💜 flash lila** de 100 ms cada vegada
que l'ESP32 rep una comanda MQTT vàlida — feedback visual immediat!

---

## ⚙️ Arquitectura (4 tasques)

```
Core 1
├── neopixelTask   (prio 3)   ← LED RGB + flash lila
├── tempInternaTask (prio 4)  ← Sensor intern
├── tempDS18B20Task (prio 4)  ← Sensor extern DS18B20
└── mainTask       (prio 5)   ← WiFi, MQTT + control LEDs
```

`mainTask` s'ha ampliat amb:
- Subscripció a `TOPIC_ORDRE` amb parsing de comandes `ledX ON/OFF/TOG`
- Subscripció a `TOPIC_LEDS` per restaurar estat en reiniciar
- Publicació de l'estat dels LEDs al JSON d'estat i al tòpic `stateLEDS`

---

## 🐛 Troubleshooting

| Símptoma | Causa | Solució |
|----------|-------|---------|
| El LED no s'encén | Càtode/ànode invertit | Potes: pota llarga = positiu (GPIO) |
| El LED no s'encén | Resistència massa alta | Prova 220 Ω en lloc de 330 Ω |
| No reb cap comanda | Prefix erroni | Comprova que `ALUMNE_ID` sigui correcte |
| Flash lila no es veu | És molt breu | És normal, dura només 100 ms |
| L'estat dels LEDs es perd | — | Ara es restaura des de `stateLEDS` retained |

---

## 📁 Fitxers

| Fitxer | Descripció |
|--------|------------|
| `Exercici-03.ino` | Codi font amb control de 4 LEDs per MQTT |
| `README.md` | Aquest document |

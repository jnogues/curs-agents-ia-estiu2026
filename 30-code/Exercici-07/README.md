# 07 — Polsadors i actuadors

Evolució de l'Exercici-06: afegeix **4 polsadors** (amb debounce per software)
i **3 actuadors** (TRIAC, relé i buzzer) controlables per MQTT.

L'ESP32 ara pot actuar sobre càrregues reals i comunicar events físics al broker.

---

## 🆕 Novetats respecte a l'Exercici-06

| Concepte | Exercici-06 | Exercici-07 |
|----------|-------------|-------------|
| Tasques FreeRTOS | 4 | **5** (+`polsadorsTask`) |
| Polsadors | — | **4** (GPIO 39–42, pull-up intern) |
| Actuadors | — | **TRIAC** (GPIO6) + **Relé** (GPIO18) + **Buzzer** (GPIO38) |
| Comandes MQTT | `ledX ON/OFF/TOG` | + `triac`, `rele`, `buzzer freq dur` |
| Estat retained | LEDs | LEDs + **TRIAC + Relé** |
| Tòpic nou | — | `/polsadors` → events de cada botó |

---

## 🛒 Material extra

- 4 × Polsador tàctil (normalment obert)
- Cables dupont

### Connexions dels polsadors

```
GPIO 39  ──── Polsador 1 ──── GND    (l'altre extrem)
GPIO 40  ──── Polsador 2 ──── GND
GPIO 41  ──── Polsador 3 ──── GND
GPIO 42  ──── Polsador 4 ──── GND
```

> 💡 El pull-up intern de l'ESP32-S3 manté la línia a HIGH en repòs.
> En prémer, la línia passa a LOW (lògica inversa).

---

## 📡 Comandes MQTT

Envia al tòpic `/esp32s3/<alumne>/ordre`:

| Comanda | Efecte |
|---------|--------|
| `led1 ON` / `OFF` / `TOG` | Control LED 1 (idem led2..4) |
| `triac ON` | Activa el TRIAC (GPIO 6) |
| `triac OFF` | Desactiva el TRIAC |
| `triac TOG` | Inverteix l'estat del TRIAC |
| `rele ON` / `OFF` / `TOG` | Idem per al relé (GPIO 18) |
| `buzzer 2000 30` | Buzzer a 2000 Hz durant 30 ms (màx 40 ms) |

### Exemples

```bash
# Activar TRIAC
mosquitto_pub -h 46.224.116.35 -t "/esp32s3/alumneXX/ordre" -m "triac ON"

# Buzzer a 1 kHz, 20 ms
mosquitto_pub -h 46.224.116.35 -t "/esp32s3/alumneXX/ordre" -m "buzzer 1000 20"

# Veure events dels polsadors en temps real
mosquitto_sub -h 46.224.116.35 -t "/esp32s3/alumneXX/polsadors" -v
```

---

## 📡 Tòpics MQTT

| Tòpic | Direcció | Retained | Contingut |
|-------|----------|----------|-----------|
| `/esp32s3/<id>/estat` | ESP32 → Broker | No | JSON cada 5 s (inclou `triac`, `rele`, `polsadors`) |
| `/esp32s3/<id>/stateLEDS` | ESP32 → Broker | **Sí** ✅ | `{"leds":[…],"triac":0,"rele":1}` |
| `/esp32s3/<id>/ordre` | Broker → ESP32 | No | Comandes LEDs + actuadors |
| `/esp32s3/<id>/online` | Broker → tots | **Sí** ✅ | `"1"` / `"0"` LWT |
| `/esp32s3/<id>/polsadors` | ESP32 → Broker | No | `{"boto":1,"event":"PREMUT"}` / `"ALLIBERAT"` |
| `/esp32s3/<id>/temperaturaINT` | ESP32 → Broker | No | Temperatura interna cada 10 s |
| `/esp32s3/<id>/temperaturaDS` | ESP32 → Broker | No | Temperatura DS18B20 cada 10 s |

### JSON d'estat resultant

```json
{
  "time": "2026-05-09 10:32:05",
  "ip": "192.168.1.5",
  "rssi": -62,
  "uptime": 124,
  "leds": [1, 0, 0, 1],
  "triac": 0,
  "rele": 1,
  "buzzer": 0,
  "polsadors": [0, 0, 1, 0]
}
```

---

## ⚙️ Arquitectura (5 tasques)

```
Core 1
├── neopixelTask    (prio 3 · 3 KB)  ← LED RGB
├── polsadorsTask   (prio 3 · 2 KB)  ← Llegeix 4 botons + debounce + publica events ⭐
├── tempInternaTask (prio 4 · 4 KB)  ← Sensor intern
├── tempDS18B20Task (prio 4 · 4 KB)  ← DS18B20
└── mainTask        (prio 5 · 8 KB)  ← WiFi, MQTT, LEDs, actuadors, buzzer timer
```

### Debounce per software

La `polsadorsTask` fa una lectura cada `DEBOUNCE_MS` (50 ms) i només confirma
un canvi d'estat si es manté estable durant tot el període:

```
Prem  ─────┐           ┌──── Allibera
            │░░░░░░░░░░│
            └──────────┘
            ←  50 ms  →
            (descarta rebots)
```

### Buzzer no bloquejant

El buzzer s'activa amb `ledcAttach()` i s'apaga automàticament a `mainTask`
quan `millis() >= buzzerEndTime` — sense bloquejar cap altra tasca.

---

## 🐛 Troubleshooting

| Símptoma | Causa | Solució |
|----------|-------|---------|
| Polsador dispara múltiples events | Rebots mecànics | Augmenta `DEBOUNCE_MS` (prova 80 ms) |
| `triac` / `rele` no responen | Cablejat GPIO incorrecte | Comprova GPIO6 (TRIAC) i GPIO18 (Relé) |
| Buzzer no sona | Freqüència fora de rang | Prova entre 500 i 4000 Hz |
| Durada del buzzer massa llarga | Límit de 40 ms al codi | El codi trunca `dur` a 40 ms per protecció |
| Estat TRIAC/Relé no es restaura | Primer missatge retained no rebut | Comprova `stateLEDS` al broker |

---

## 📁 Fitxers

| Fitxer | Descripció |
|--------|------------|
| `Exercici-07.ino` | Codi font amb polsadors i actuadors |
| `README.md` | Aquest document |

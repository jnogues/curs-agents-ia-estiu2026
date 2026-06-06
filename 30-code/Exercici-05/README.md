# 05 — NTP, ArduinoJson i mutex

Evolució de l'Exercici-04: afegeix **hora real via NTP**, **parsing JSON robust**
amb ArduinoJson, i **mutex** per protegir les variables compartides entre tasques.

El missatge d'estat ara inclou el camp `"time"` amb la data i hora actuals.

---

## 🆕 Novetats respecte a l'Exercici-04

| Concepte | Exercici-04 | Exercici-05 |
|----------|-------------|-------------|
| Hora real | — | ✅ **NTP** (`pool.ntp.org`) |
| JSON `estat` | `{"ssid":…,"ip":…,…}` | + `"time":"2026-05-09 10:32:05"` |
| Parsing retained | Manual (indexOf) | ✅ **ArduinoJson** |
| Variables compartides | `volatile` | ✅ **Mutex** (`dataMutex`) |
| Flash Neopixel | Bloquejant (`vTaskDelay`) | ✅ No bloquejant (`millis()`) |
| Llibreria nova | — | **ArduinoJson** |

### JSON d'estat resultant (cada 5 s)

```json
{
  "time":    "2026-05-09 10:32:05",
  "ip":      "192.168.1.5",
  "rssi":    -62,
  "uptime":  124,
  "leds":    [1, 0, 0, 1]
}
```

---

## 🧰 Llibreria nova

Instal·lar des de l'**Library Manager** (`Eines → Gestiona llibreries…`):

| Llibreria | Autor |
|-----------|-------|
| **ArduinoJson** | Benoit Blanchon |

---

## 🕐 Configuració NTP

Al codi pots ajustar el fus horari:

```cpp
#define NTP_SERVER      "pool.ntp.org"
#define GMT_OFFSET_SEC  3600    // UTC+1 (hivern)
#define DST_OFFSET_SEC  3600    // +1 h addicional en horari d'estiu → UTC+2
```

Per a UTC+0 sense canvi d'horari: tots dos a `0`.

---

## 🔒 Per què el mutex?

A partir d'aquest exercici, `mainTask` i els callbacks MQTT s'executen en fils
(*tasks*) separats. Sense protecció, dos fils podrien llegir i escriure
`ledState[]` o `estatActual` alhora i obtenir valors corromputs.

El `dataMutex` garanteix que només una tasca accedeix a les variables
compartides en cada moment:

```cpp
if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50))) {
    ledState[idx] = true;          // escriptura segura
    xSemaphoreGive(dataMutex);
}
```

---

## 🚀 Configuració

Obre `Exercici-05.ino` i canvia el teu identificador (i el WiFi si cal):

```cpp
#define ALUMNE_ID    "alumneXX"

#define WIFI_SSID_1  "SSID_ALUMNE"
#define WIFI_PASS_1  "PASSWORD_ALUMNE"
```

El Monitor Sèrie (115200 baud) ha de mostrar:

```
=== ESP32-S3 Template v5 ===
[WiFi] Connectat a 'NomXarxa'  IP: 192.168.x.x
[NTP] Hora: 2026-05-09 10:32:00
[MQTT] Connectat al broker
[LEDS] Estat restaurat des del broker
[MQTT TX] {"time":"2026-05-09 10:32:05","ip":"192.168.x.x","rssi":-62,"uptime":5,"leds":[0,0,0,0]}
```

---

## 📡 Tòpics MQTT

| Tòpic | Direcció | Retained | Contingut |
|-------|----------|----------|-----------|
| `/esp32s3/<id>/estat` | ESP32 → Broker | No | JSON amb `time`, `ip`, `rssi`, `uptime`, `leds` cada 5 s |
| `/esp32s3/<id>/stateLEDS` | ESP32 → Broker | **Sí** ✅ | `{"leds":[1,0,0,1]}` |
| `/esp32s3/<id>/ordre` | Broker → ESP32 | No | Comandes `ledX ON/OFF/TOG` |
| `/esp32s3/<id>/temperaturaINT` | ESP32 → Broker | No | Temperatura interna cada 10 s |
| `/esp32s3/<id>/temperaturaDS` | ESP32 → Broker | No | Temperatura DS18B20 cada 10 s |

---

## ⚙️ Arquitectura (4 tasques)

```
Core 1
├── neopixelTask    (prio 3 · 3 KB)  ← Flash lila no bloquejant (millis)
├── tempInternaTask (prio 4 · 4 KB)  ← Sensor intern → MQTT
├── tempDS18B20Task (prio 4 · 4 KB)  ← DS18B20 → MQTT
└── mainTask        (prio 5 · 8 KB)  ← WiFi, NTP, MQTT, LEDs
```

El `dataMutex` es crea a `setup()` **abans** de crear les tasques — si es creés
dins d'una tasca, l'altra podria intentar usar-lo abans que existís.

---

## 🐛 Troubleshooting

| Símptoma | Causa | Solució |
|----------|-------|---------|
| `time` mostra `--:--:--` | NTP no sincronitzat | Comprova accés a internet; el codi continua sense hora |
| `time` mostra hora incorrecta | Fus horari | Ajusta `GMT_OFFSET_SEC` i `DST_OFFSET_SEC` |
| Error de compilació | Falta ArduinoJson | Instal·la **ArduinoJson** de Benoit Blanchon |
| Estat LEDs no es restaura | JSON retained mal format | Comprova amb `mosquitto_sub … -t "…/stateLEDS" -C 1` |

---

## 📁 Fitxers

| Fitxer | Descripció |
|--------|------------|
| `Exercici-05.ino` | Codi font amb NTP, ArduinoJson i mutex |
| `README.md` | Aquest document |

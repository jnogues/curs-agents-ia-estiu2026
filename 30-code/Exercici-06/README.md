# 06 — LWT: presència MQTT

Evolució de l'Exercici-05: afegeix el mecanisme **LWT** (*Last Will and Testament*)
de MQTT. Permet saber, des de qualsevol subscriptor, si el dispositiu està en línia
o ha caigut — fins i tot si cau de forma inesperada (tall de corrent, crash).

---

## 🆕 Novetats respecte a l'Exercici-05

| Concepte | Exercici-05 | Exercici-06 |
|----------|-------------|-------------|
| Tòpic `online` | — | ✅ `/online` retained |
| En connectar | — | Publica `"1"` a `/online` |
| En caure (LWT) | — | Broker publica `"0"` automàticament |
| Hardware nou | — | Cap |
| Llibreries noves | — | Cap |

---

## 📡 Com funciona el LWT

El LWT (*Last Will and Testament*) és un missatge que el **broker** publica
automàticament quan detecta que el client s'ha desconnectat de forma inesperada
(sense fer un `DISCONNECT` net).

```
Dispositiu        Broker MQTT          Subscriptor
    │                  │                    │
    │── CONNECT ───────►│                   │
    │   (willTopic,     │                   │
    │    willMsg="0")   │                   │
    │◄─ CONNACK ────────│                   │
    │                   │                   │
    │── publish "1" ───►│── retain "1" ────►│  ← "estic en línia"
    │                   │                   │
    │   [tall de llum]  │                   │
    │✗                  │                   │
    │                   │── retain "0" ────►│  ← "ha caigut!"
    │                   │  (LWT automàtic)  │
```

---

## 🚀 Configuració

Obre `Exercici-06.ino` i canvia el teu identificador (i el WiFi si cal):

```cpp
#define ALUMNE_ID    "alumneXX"

#define WIFI_SSID_1  "SSID_ALUMNE"
#define WIFI_PASS_1  "PASSWORD_ALUMNE"
```

Per veure la presència en temps real:

```bash
mosquitto_sub -h 46.224.116.35 -t "/esp32s3/alumneXX/online" -v
```

Hauries de veure `"1"` en connectar i `"0"` en desconnectar (o en treure la
alimentació sense tancar bé la connexió).

---

## 📡 Tòpics MQTT

| Tòpic | Direcció | Retained | Contingut |
|-------|----------|----------|-----------|
| `/esp32s3/<id>/estat` | ESP32 → Broker | No | JSON complet cada 5 s |
| `/esp32s3/<id>/stateLEDS` | ESP32 → Broker | **Sí** ✅ | Estat LEDs |
| `/esp32s3/<id>/ordre` | Broker → ESP32 | No | Comandes `ledX ON/OFF/TOG` |
| `/esp32s3/<id>/temperaturaINT` | ESP32 → Broker | No | Temperatura interna cada 10 s |
| `/esp32s3/<id>/temperaturaDS` | ESP32 → Broker | No | Temperatura DS18B20 cada 10 s |
| `/esp32s3/<id>/online` | Broker → tots | **Sí** ✅ | `"1"` en línia · `"0"` LWT |

> 💡 El tòpic `online` és **retained**: qualsevol subscriptor que es connecti
> més tard rebrà l'últim valor sense esperar el proper missatge.

---

## 🔧 Implementació al codi

Dues línies clau:

```cpp
// 1. Registrar el LWT abans de connectar (setup de la config MQTT)
mqttCfg.willTopic = TOPIC_ONLINE;   // broker publica "0" si el client cau

// 2. Publicar "1" en cada (re)connexió exitosa
mqtt.onConnect([]() {
    mqtt.publish(TOPIC_ONLINE, "1", true);   // retained=true
    ...
});
```

La valeur per defecte del LWT (el missatge que el broker publica) és `"0"` —
definit per la llibreria MycilaMQTT quan `willTopic` és diferent de buit.

---

## ⚙️ Arquitectura (4 tasques — sense canvis)

```
Core 1
├── neopixelTask    (prio 3 · 3 KB)
├── tempInternaTask (prio 4 · 4 KB)
├── tempDS18B20Task (prio 4 · 4 KB)
└── mainTask        (prio 5 · 8 KB)  ← + publica "1" a /online en connectar
                                         + willTopic configurat per al LWT
```

---

## 🐛 Troubleshooting

| Símptoma | Causa | Solució |
|----------|-------|---------|
| `/online` sempre a `"0"` | El dispositiu no connecta | Comprova el LED (blau = WiFi, groc = MQTT) |
| `/online` no canvia a `"0"` en desconnectar | Desconnexió neta (no LWT) | El LWT s'activa quan la connexió cau sense `DISCONNECT` — prova a treure la llum |
| Cap tòpic `/online` al subscriptor | No hi ha cap missatge retained encara | Arrenca l'ESP32 primer, després subscriu-te |

---

## 📁 Fitxers

| Fitxer | Descripció |
|--------|------------|
| `Exercici-06.ino` | Codi font amb LWT MQTT |
| `README.md` | Aquest document |

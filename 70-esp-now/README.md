# ESP-NOW ↔ MQTT Gateway

Projecte per a curs. Xarxa de sensors/actuadors basada en ESP-NOW amb pont MQTT.

## Arquitectura

```
[NodeMCU ESP8266]  ──┐
[NodeMCU ESP8266]  ──┤  ESP-NOW (broadcast)  ┌──[ESP32-S3 Gateway]──WiFi──[Broker MQTT]
[NodeMCU ESP8266]  ──┘                        └── (LittleFS: registre de nodes)
```

## Hardware

| Rol     | Board             | FQBN                        |
|---------|-------------------|-----------------------------|
| Gateway | ESP32-S3 DevKitC-1 | `esp32:esp32:esp32s3`       |
| Node    | NodeMCU ESP8266 1.0 | `esp8266:esp8266:nodemcuv2` |

### Connexions node

| Component       | NodeMCU | GPIO  | Notes                                     |
|-----------------|---------|-------|-------------------------------------------|
| LM75B SDA       | D2      | 4     | I2C                                       |
| LM75B SCL       | D1      | 5     | I2C                                       |
| LM75B A0/A1/A2  | GND     | —     | Adreça I2C = 0x48                         |
| LED (positiu)   | D7      | 13    | HIGH=ON · via resistència ~220Ω           |
| LED (negatiu)   | D3      | 0     | LOW=ON · boot pin, apagat per defecte     |
| LED (negatiu)   | D4      | 2     | LOW=ON · boot pin, apagat per defecte     |
| NeoPixel data   | D8      | 15    | WS2812, 1 píxel · boot pin LOW (ok)       |
| Polsador 1      | D6      | 12    | Pull-down extern · HIGH quan premut       |
| Polsador 2      | D5      | 14    | Pull-down extern · HIGH quan premut       |
| Potenciòmetre   | A0      | ADC   | 0–1023 (0–3.3V en NodeMCU)               |
| LED integrat    | D0      | 16    | LOW=ON · heartbeat 100ms                  |

## Estructura del projecte

```
esp-now/
├── shared/
│   └── message.h           ← protocol comú (symlinks als altres directoris)
├── gateway/
│   ├── gateway.ino         ← sketch ESP32-S3
│   └── message.h           ← symlink → shared/message.h
└── node_esp8266/
    ├── node_esp8266.ino    ← sketch NodeMCU ESP8266
    └── message.h           ← symlink → shared/message.h
```

## Prerequisits

```bash
# Instal·lar arduino-cli
curl -fsSL https://downloads.arduino.cc/arduino-cli/arduino-cli_latest_Linux_64bit.tar.gz \
  -o /tmp/arduino-cli.tar.gz && tar -xzf /tmp/arduino-cli.tar.gz -C ~/.local/bin arduino-cli

# Afegir plataformes i actualitzar índex
arduino-cli config add board_manager.additional_urls \
  https://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli config add board_manager.additional_urls \
  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli core update-index

# Instal·lar cores
arduino-cli core install esp32:esp32
arduino-cli core install esp8266:esp8266

# Instal·lar llibreries
arduino-cli lib install "ArduinoJson" "PubSubClient" "Adafruit NeoPixel"
```

## Configuració

### Gateway (`gateway/gateway.ino`)

```c
#define WIFI_SSID    "la-teva-xarxa"
#define WIFI_PASS    "la-teva-contrasenya"
#define MQTT_SERVER  "ip-del-broker"
#define MQTT_PORT    1883
```

### Node (`node_esp8266/node_esp8266.ino`)

```c
#define NODE_NAME       "salon"           // nom llegible
#define GATEWAY_CHANNEL 1                 // canal WiFi del router (imprès per la gateway en arrencar)
#define GATEWAY_MAC     {0xAA,0xBB,...}   // MAC de la gateway (impresa per la gateway en arrencar)
```

**Com obtenir la MAC i el canal de la gateway:**
```bash
arduino-cli monitor --port /dev/ttyACM0 --config baudrate=115200
# Buscar la línia: [WIFI] Connectat. IP: ...  Canal: X  MAC: AA:BB:CC:DD:EE:FF
```

## Compilar i flashejar

```bash
# Gateway
arduino-cli compile --fqbn esp32:esp32:esp32s3 gateway/
arduino-cli upload  --fqbn esp32:esp32:esp32s3 --port /dev/ttyACM0 gateway/

# Node
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 node_esp8266/
arduino-cli upload  --fqbn esp8266:esp8266:nodemcuv2 --port /dev/ttyUSB0 node_esp8266/
```

## Protocol de missatges

Format JSON, màxim 250 bytes (limitació hardware ESP-NOW).

| `type`     | Direcció      | Descripció                        |
|------------|---------------|-----------------------------------|
| `announce` | node → gateway | Primer contacte, capacitats       |
| `sensor`   | node → gateway | Dades de sensors (periòdic)       |
| `event`    | node → gateway | Esdeveniment puntual              |
| `state`    | node → gateway | Estat actual d'un actuador        |
| `command`  | gateway → node | Ordre                             |
| `ack`      | node → gateway | Confirmació de comanda            |

## Topics MQTT

| Topic                  | Direcció | Contingut                  |
|------------------------|----------|----------------------------|
| `espnow/{id}/state`    | publica  | payload de sensor/event    |
| `espnow/{id}/set`      | subscriu | comanda per enviar al node |

## Decisions tècniques destacades

**Broadcast obligatori des dels nodes ESP8266:**
El driver WiFi de l'ESP32 en mode `WIFI_STA` no pot respondre ACKs
de la capa MAC a paquets unicast de dispositius externs a l'AP. La solució
és que els nodes enviïn a `FF:FF:FF:FF:FF:FF`. La gateway identifica l'origen
pel camp `id` del JSON.

**Power save desactivat a la gateway:**
Amb `esp_wifi_set_ps(WIFI_PS_NONE)`, el ràdio no dorm entre balises de l'AP.
Sense això, el driver descarta silenciosament els paquets ESP-NOW rebuts durant
els intervals de son.

**Canal forçat als nodes ESP8266:**
Un ESP8266 sense WiFi connectat no té canal fix. Es crida `wifi_set_channel()`
*després* de `esp_now_init()` (que pot canviar el canal) per garantir que
node i gateway operen al mateix canal.

**Node ID amb MAC:**
Format `{nom}_{3 últims bytes MAC}` (ex: `salon_AABBCC`). El nom és
llegible i configurable; els bytes de MAC garanteixen unicitat sense
necessitat de coordinació central.

## Indicadors visuals

### Node (GPIO16 — LED integrat D0)

| Comportament | Significat |
|---|---|
| Parpelleig ràpid 100ms | Node operatiu (heartbeat) |

### Gateway (NeoPixel integrat GPIO48)

| Color | Durada | Significat |
|---|---|---|
| Ambre tènue | Pols 80ms cada 1s | Gateway operativa (heartbeat) |
| Verd | 150ms | Paquet ESP-NOW rebut d'un node |
| Blau | 150ms | Comanda ESP-NOW enviada correctament |

## Verificar funcionament

### Escoltar dades del sensor
```bash
mosquitto_sub -h IP_BROKER -t "espnow/#" -v
```
Hauries de veure cada 10 segons:
```
espnow/node1_FB590C/state {"id":"node1_FB590C","type":"sensor","data":{"temp":27.63}}
```

### Controlar els LEDs
```bash
# LED GPIO13 (lògica positiva)
mosquitto_pub -h IP_BROKER -t "espnow/node1_FB590C/set" -m '{"target":"led13","state":"on"}'
mosquitto_pub -h IP_BROKER -t "espnow/node1_FB590C/set" -m '{"target":"led13","state":"off"}'

# LED GPIO0 (lògica negativa)
mosquitto_pub -h IP_BROKER -t "espnow/node1_FB590C/set" -m '{"target":"led0","state":"on"}'
mosquitto_pub -h IP_BROKER -t "espnow/node1_FB590C/set" -m '{"target":"led0","state":"off"}'

# LED GPIO2 (lògica negativa)
mosquitto_pub -h IP_BROKER -t "espnow/node1_FB590C/set" -m '{"target":"led2","state":"on"}'
mosquitto_pub -h IP_BROKER -t "espnow/node1_FB590C/set" -m '{"target":"led2","state":"off"}'

# NeoPixel — color RGB
mosquitto_pub -h IP_BROKER -t "espnow/node1_FB590C/set" -m '{"target":"neo","r":255,"g":0,"b":0}'
mosquitto_pub -h IP_BROKER -t "espnow/node1_FB590C/set" -m '{"target":"neo","state":"off"}'
```

### Events dels polsadors
El node publica automàticament quan es prem un polsador:
```
espnow/node1_FB590C/event {"id":"...","type":"event","data":{"trigger":"btn12","state":"pressed"}}
espnow/node1_FB590C/event {"id":"...","type":"event","data":{"trigger":"btn14","state":"pressed"}}
```

### Dades del sensor
```
espnow/node1_FB590C/state {"id":"...","type":"sensor","data":{"temp":27.63,"pot":512}}
```

El node respon sempre amb un `ack` a cada comanda rebuda.

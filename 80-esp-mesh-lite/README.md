# painlessMesh ↔ MQTT Bridge

Projecte per a curs. Xarxa mesh Wi-Fi amb pont MQTT basat en painlessMesh.

## Arquitectura

```
[NodeMCU ESP8266]  ──┐
[NodeMCU ESP8266]  ──┤  painlessMesh (Wi-Fi)  [ESP32-S3 Bridge] ──WiFi── [Broker MQTT]
[NodeMCU ESP8266]  ──┘         AP+STA                AP + STA
```

El bridge és el node arrel de la mesh. Els nodes es connecten directament a ell
o entre ells (multi-salt). No calen credencials WiFi als nodes.

El bridge corre dues tasques FreeRTOS independents:

```
Core 0  meshTask     — mesh.update() + envia comandes de la cua
Core 1  wifiMqttTask — WiFi/MQTT amb backoff + publica missatges de la cua

meshRxQueue (10 msg) ── mesh rebut ──► publicar a MQTT
mqttTxQueue ( 5 msg) ── comanda MQTT ──► broadcast a la mesh
```

## Hardware

| Rol    | Board                | FQBN                          |
|--------|----------------------|-------------------------------|
| Bridge | ESP32-S3 DevKitC-1   | `esp32:esp32:esp32s3`         |
| Node   | NodeMCU ESP8266 1.0  | `esp8266:esp8266:nodemcuv2`   |

### Connexions node ESP8266

| Component      | NodeMCU | GPIO | Notes                                  |
|----------------|---------|------|----------------------------------------|
| LM75B SDA      | D2      | 4    | I2C                                    |
| LM75B SCL      | D1      | 5    | I2C                                    |
| LM75B A0/A1/A2 | GND     | —    | Adreça I2C = 0x48                      |
| LED (positiu)  | D7      | 13   | HIGH=ON · via resistència ~220Ω        |
| LED (negatiu)  | D3      | 0    | LOW=ON · boot pin, apagat per defecte  |
| LED (negatiu)  | D4      | 2    | LOW=ON · boot pin, apagat per defecte  |
| NeoPixel data  | D8      | 15   | WS2812, 1 píxel · boot pin LOW (ok)   |
| Polsador 1     | D6      | 12   | Pull-down extern · HIGH quan premut    |
| Polsador 2     | D5      | 14   | Pull-down extern · HIGH quan premut    |
| Potenciòmetre  | A0      | ADC  | 0–1023 (0–3.3V en NodeMCU)            |
| LED integrat   | D0      | 16   | LOW=ON · heartbeat 100ms               |

## Indicadors visuals

### Node (GPIO16 — LED integrat D0)

| Comportament       | Significat       |
|--------------------|------------------|
| Parpelleig 100ms   | Node operatiu    |

### Bridge (NeoPixel integrat GPIO48)

| Color       | Durada           | Significat                        |
|-------------|------------------|-----------------------------------|
| Ambre tènue | Pols 80ms cada 1s | Bridge operatiu (heartbeat)      |
| Verd        | 150ms            | Missatge mesh rebut d'un node     |
| Blau        | 150ms            | Comanda MQTT reenviada a la mesh  |

## Estructura del projecte

```
esp-mesh-lite/
├── basic_node/            ← sketch de demostració (corre als dos boards)
├── bridge/                ← sketch ESP32-S3: pont mesh ↔ MQTT
├── mesh_node/             ← sketch NodeMCU ESP8266: sensors + actuadors
└── firmware/
    ├── primer-vermell/    ← binari precompilat per al primer node
    └── segon-negre/       ← binari precompilat per al segon node
```

## Prerequisits

```bash
# Cores (mateixos que el projecte ESP-NOW)
arduino-cli core install esp32:esp32
arduino-cli core install esp8266:esp8266

# Llibreries
arduino-cli lib install "Painless Mesh" "TaskScheduler" "ArduinoJson" \
                        "PubSubClient" "Adafruit NeoPixel"

# Dependències no al registre oficial (cal habilitar unsafe install)
arduino-cli config set library.enable_unsafe_install true
arduino-cli lib install --git-url https://github.com/ESP32Async/AsyncTCP.git
arduino-cli lib install --git-url https://github.com/ESP32Async/ESPAsyncTCP.git
```

## Configuració

### Bridge (`bridge/bridge.ino`)

```c
#define WIFI_SSID   "la-teva-xarxa"
#define WIFI_PASS   "la-teva-contrasenya"
#define MQTT_SERVER "ip-del-broker"
#define MQTT_PORT   1883

#define MESH_SSID   "mesh-lab"       // ha de coincidir amb els nodes
#define MESH_PASS   "meshpass123"    // ha de coincidir amb els nodes
```

### Node (`mesh_node/mesh_node.ino`)

```c
#define NODE_NAME  "salon"      // nom llegible; es combina amb 3 bytes de MAC
#define MESH_SSID  "mesh-lab"   // ha de coincidir amb el bridge
#define MESH_PASS  "meshpass123"
```

No cal cap credencial WiFi ni MAC del bridge — la mesh s'autoorganitza.

## Compilar i flashejar

```bash
# Bridge
arduino-cli compile --fqbn esp32:esp32:esp32s3 bridge/
arduino-cli upload  --fqbn esp32:esp32:esp32s3 --port /dev/ttyACM0 bridge/
```

### Afegir un nou node

Cada node necessita un binari amb el seu `NODE_NAME`. El flux és:

```bash
# 1. Canviar NODE_NAME al sketch
#    mesh_node/mesh_node.ino → #define NODE_NAME "cuina"

# 2. Compilar i desar el binari
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 \
  --output-dir firmware/cuina mesh_node/

# 3. Restaurar NODE_NAME a "node1" al sketch

# 4. Connectar el node i flashejar des del binari desat
arduino-cli upload --fqbn esp8266:esp8266:nodemcuv2 \
  --port /dev/ttyUSB0 --input-dir firmware/cuina
```

El binari queda a `firmware/cuina/` — si cal reflashejar el mateix node, el pas 4 és suficient.

## Topics MQTT

| Topic                   | Direcció | Contingut                              |
|-------------------------|----------|----------------------------------------|
| `mesh/{node_id}/state`  | publica  | Dades de sensor                        |
| `mesh/{node_id}/event`  | publica  | Esdeveniments de polsadors             |
| `mesh/{node_id}/ack`    | publica  | Confirmació de comanda executada       |
| `mesh/{node_id}/status` | publica  | Canvi d'estat del node (offline, etc.) |
| `mesh/bridge/status`    | publica  | Estat del bridge cada 30s              |
| `mesh/{node_id}/set`    | subscriu | Comanda per al node                    |

El `node_id` té el format `{NODE_NAME}_{3 últims bytes MAC}` (ex: `node1_FB590C`).
Cada node obté un ID únic automàticament — no cal cap configuració addicional per afegir nodes.

Nodes del laboratori:

| Node ID          | Hardware             | Notes          |
|------------------|----------------------|----------------|
| `primer-vermell_FB590C` | NodeMCU ESP8266 1.0  | Primer node  |
| `segon-negre_17B8C4`   | NodeMCU ESP8266 1.0  | Segon node   |

## Format de missatges

Tots els missatges són JSON de fins a 250 bytes.

### Node → bridge → MQTT

```json
// Sensor (cada 10s) — amb timestamp en ms sincronitzat a tota la mesh
{"id":"primer-vermell_FB590C","type":"sensor","ts":91076,"data":{"temp":28.63,"pot":632}}

// Event polsador
{"id":"primer-vermell_FB590C","type":"event","ts":91500,"data":{"trigger":"btn12","state":"pressed"}}

// ACK de comanda executada
{"id":"primer-vermell_FB590C","type":"ack","ts":91852,"cmd":"led13"}

// Notificació offline (publicada pel bridge quan un node es desconnecta)
{"id":"primer-vermell_FB590C","type":"status","data":{"status":"offline"}}

// Estat del bridge (cada 30s)
{"uptime_s":540,"mesh_nodes":2,"free_heap":150044}
```

### MQTT → bridge → node (comandes)

```bash
# LED GPIO13
mosquitto_pub -h IP_BROKER -t "mesh/node1_FB590C/set" \
  -m '{"id":"node1_FB590C","type":"command","data":{"target":"led13","state":"on"}}'

# LED GPIO0
mosquitto_pub -h IP_BROKER -t "mesh/node1_FB590C/set" \
  -m '{"id":"node1_FB590C","type":"command","data":{"target":"led0","state":"on"}}'

# LED GPIO2
mosquitto_pub -h IP_BROKER -t "mesh/node1_FB590C/set" \
  -m '{"id":"node1_FB590C","type":"command","data":{"target":"led2","state":"on"}}'

# NeoPixel — color RGB
mosquitto_pub -h IP_BROKER -t "mesh/node1_FB590C/set" \
  -m '{"id":"node1_FB590C","type":"command","data":{"target":"neo","r":255,"g":0,"b":0}}'

# NeoPixel — apagar
mosquitto_pub -h IP_BROKER -t "mesh/node1_FB590C/set" \
  -m '{"id":"node1_FB590C","type":"command","data":{"target":"neo","state":"off"}}'
```

## Escoltar totes les dades

```bash
mosquitto_sub -h IP_BROKER -t "mesh/#" -v
```

Exemple amb dos nodes actius:
```
mesh/primer-vermell_FB590C/state  {"id":"primer-vermell_FB590C","type":"sensor","ts":91076,"data":{"temp":28.63,"pot":632}}
mesh/segon-negre_17B8C4/state     {"id":"segon-negre_17B8C4","type":"sensor","ts":134935,"data":{"temp":28.88,"pot":581}}
mesh/primer-vermell_FB590C/ack    {"id":"primer-vermell_FB590C","type":"ack","ts":91852,"cmd":"led13"}
mesh/bridge/status                {"uptime_s":540,"mesh_nodes":2,"free_heap":150044}
```

## Resiliència del bridge

| Escenari | Comportament |
|---|---|
| Router no disponible en arrencar | Timeout 15s → arrenca amb canal per defecte 1 |
| Router cau en marxa | `wifiMqttTask` reintenta cada 30s (backoff) |
| Broker cau en marxa | `wifiMqttTask` reintenta cada 5s (backoff) |
| WiFi o MQTT caiguts | `meshTask` segueix funcionant al Core 0 |
| Broker caigut temporalment | Fins a 10 missatges bufferitzats a `meshRxQueue` |

## Decisions tècniques destacades

**Tasques FreeRTOS independents:**
La mesh corre al Core 0 (`meshTask`) i mai queda bloquejada per problemes de
xarxa. WiFi i MQTT es gestionen al Core 1 (`wifiMqttTask`) amb backoff independent.
La comunicació entre tasques es fa exclusivament via cues FreeRTOS (thread-safe),
sense variables compartides ni mutexos addicionals.

**Inicialització del bridge en tres passos:**
`mesh.init()` sobreescriu el mode WiFi internament. Per evitar perdre la connexió
al router, el bridge (1) connecta al router per obtenir el canal, (2) inicialitza
la mesh en mode `WIFI_AP` al mateix canal, (3) restaura la STA i reconnecta al router.
Tots dos interfaces (mesh AP i router STA) operen al mateix canal perquè l'ESP32
comparteix la ràdio.

**Bridge en mode `WIFI_AP` (no `WIFI_AP_STA`):**
El bridge és el node arrel de la mesh — no necessita connectar-se a cap node pare.
Inicialitzar en `WIFI_AP` deixa la interfície STA lliure per al router.
En `WIFI_AP_STA`, painlessMesh usaria la STA per cercar nodes pare i entraria
en conflicte amb la connexió al router.

**Comandes via broadcast:**
El bridge fa broadcast de les comandes MQTT a tota la mesh. Cada node filtra
pel camp `id` del JSON i ignora les comandes que no li pertoquen. Elimina
la necessitat de mantenir una taula nodeId↔meshId al bridge.

**Timestamps sincronitzats (`ts`):**
painlessMesh sincronitza un rellotge comú a tots els nodes. `mesh.getNodeTime()`
retorna microsegons des de l'arrencada de la mesh, iguals a tots els nodes.
Dividit per 1000 dona mil·lisegons. Permet correlacionar events de nodes diferents
sense NTP ni rellotge extern. Overflow cada ~71 minuts (uint32_t).

**Detecció de nodes offline:**
El bridge manté un registre `meshId → nodeId llegible` que s'omple quan arriben
missatges. En `onDroppedConnection`, busca el nom al registre i posa un missatge
`{"status":"offline"}` a la cua perquè `wifiMqttTask` el publiqui. Si el node mai
havia enviat cap missatge, s'usa l'ID numèric com a fallback.

**ACK de comandes:**
El node confirma cada comanda amb un missatge `type:"ack"` que inclou el camp
`cmd` amb el target executat. Permet al sistema verificar que la comanda ha
arribat i s'ha processat, sense necessitat de polling.

**Doble `neo.show()` al node ESP8266:**
El driver WiFi de l'ESP8266 pot interrompre la transmissió bit-bang del WS2812
durant el primer `show()`. Un segon `show()` immediat garanteix que el color
s'aplica correctament.

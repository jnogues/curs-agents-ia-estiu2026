/*
  bridge.ino — Pont mesh ↔ MQTT amb FreeRTOS
  ============================================
  Rol: connecta la xarxa painlessMesh amb un broker MQTT.
  Board: esp32:esp32:esp32s3  (ESP32-S3 DevKitC-1)

  Arquitectura FreeRTOS:
    meshTask     (Core 0, prioritat 2) — mesh.update() + envia comandes de la cua
    wifiMqttTask (Core 1, prioritat 1) — WiFi + MQTT amb backoff + publica missatges

  Cues de comunicació entre tasques:
    meshRxQueue  (10 missatges) — mesh rebut → publicar a MQTT
    mqttTxQueue  ( 5 missatges) — comanda MQTT → broadcast a la mesh

  Resiliència:
    - WiFi caigut en arrencar: timeout 15s, arrenca amb canal per defecte 1
    - WiFi caigut en marxa: reintenta cada 30s (backoff)
    - Broker caigut: reintenta cada 5s (backoff)
    - Mesh segueix funcionant independentment de WiFi/MQTT
    - Missatges bufferitzats a la cua durant caigudes del broker (fins a 10)

  LED d'estat (NeoPixel WS2812 GPIO48):
    Ambre tènue  pols 80ms cada 1s   — bridge operatiu (heartbeat)
    Verd         150ms               — missatge mesh rebut
    Blau         150ms               — comanda MQTT reenviada a la mesh

  Compilar i flashejar:
    arduino-cli compile --fqbn esp32:esp32:esp32s3 bridge/
    arduino-cli upload  --fqbn esp32:esp32:esp32s3 --port /dev/ttyACM0 bridge/
*/

#include <WiFi.h>
#include <painlessMesh.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <map>

// --- Configuració ---
#define WIFI_SSID    "la-teva-xarxa"
#define WIFI_PASS    "la-teva-contrasenya"
#define MQTT_SERVER  "ip-del-broker"
#define MQTT_PORT    1883
#define MQTT_PREFIX  "mesh"

#define MESH_SSID    "mesh-lab"
#define MESH_PASS    "meshpass123"
#define MESH_PORT    5555

// Temporitzadors de reconnexió
#define WIFI_CONNECT_TIMEOUT  15000   // ms esperant WiFi en arrencar
#define WIFI_RETRY_MS         30000   // backoff WiFi caigut en marxa
#define MQTT_RETRY_MS          5000   // backoff broker caigut

// LED d'estat
#define GW_NEO_PIN           48
#define GW_FLASH_MS         150
#define GW_HEARTBEAT_MS    1000
#define GW_HEARTBEAT_PULSE   80
#define GW_COLOR_RX_R   0, 150,   0
#define GW_COLOR_TX_R   0,   0, 150
#define GW_COLOR_HB_R  15,   8,   0

// Cues FreeRTOS
#define MSG_MAX_LEN         250
#define MESH_RX_QUEUE_LEN    10
#define MQTT_TX_QUEUE_LEN     5

struct MeshRxMsg {
    uint32_t from;
    char     payload[MSG_MAX_LEN + 1];
};

struct MqttTxMsg {
    char payload[MSG_MAX_LEN + 1];
};

// --- Globals ---
Scheduler       userScheduler;
painlessMesh    mesh;
WiFiClient      wifiClient;
PubSubClient    mqtt(wifiClient);
Adafruit_NeoPixel gwNeo(1, GW_NEO_PIN, NEO_GRB + NEO_KHZ800);

QueueHandle_t meshRxQueue;
QueueHandle_t mqttTxQueue;

// Registre meshId → nodeId llegible (actualitzat des de meshTask)
std::map<uint32_t, String> nodeRegistry;
volatile uint32_t meshNodeCount = 0;

// Flash d'estat: escrit des de qualsevol tasca, llegit des de wifiMqttTask
volatile bool    flashPending = false;
volatile uint8_t pendingR = 0, pendingG = 0, pendingB = 0;
unsigned long    flashUntil = 0;
unsigned long    heartbeatNext = 0;
unsigned long    heartbeatPulseUntil = 0;

// --- NeoPixel (cridat des de wifiMqttTask) ---
void neoOff() { gwNeo.clear(); gwNeo.show(); }

void neoSet(uint8_t r, uint8_t g, uint8_t b) {
    gwNeo.setPixelColor(0, gwNeo.Color(r, g, b));
    gwNeo.show();
}

void flashSet(uint8_t r, uint8_t g, uint8_t b) {
    pendingR = r; pendingG = g; pendingB = b;
    flashPending = true;
}

void updateNeo() {
    unsigned long now = millis();
    if (flashPending) {
        uint8_t r = pendingR, g = pendingG, b = pendingB;
        flashPending = false;
        flashUntil = now + GW_FLASH_MS;
        heartbeatPulseUntil = 0;
        neoSet(r, g, b);
        return;
    }
    if (flashUntil > 0) {
        if (now < flashUntil) return;
        flashUntil = 0; neoOff(); return;
    }
    if (heartbeatPulseUntil > 0) {
        if (now >= heartbeatPulseUntil) { heartbeatPulseUntil = 0; neoOff(); }
        return;
    }
    if (now >= heartbeatNext) {
        heartbeatNext = now + GW_HEARTBEAT_MS;
        heartbeatPulseUntil = now + GW_HEARTBEAT_PULSE;
        neoSet(GW_COLOR_HB_R);
    }
}

// --- MQTT (cridat des de wifiMqttTask) ---
void mqttPublish(uint32_t from, const char *payload) {
    StaticJsonDocument<250> doc;
    const char *nodeKey = nullptr;
    const char *type    = nullptr;
    char fromStr[16];

    if (deserializeJson(doc, payload) == DeserializationError::Ok) {
        if (doc["id"])   nodeKey = doc["id"];
        if (doc["type"]) type    = doc["type"];
    }
    if (!nodeKey) {
        snprintf(fromStr, sizeof(fromStr), "%u", from);
        nodeKey = fromStr;
    }

    // Subtòpic segons el tipus de missatge
    const char *sub = "state";
    if (type) {
        if      (strcmp(type, "event")  == 0) sub = "event";
        else if (strcmp(type, "ack")    == 0) sub = "ack";
        else if (strcmp(type, "status") == 0) sub = "status";
    }

    char topic[64];
    snprintf(topic, sizeof(topic), "%s/%s/%s", MQTT_PREFIX, nodeKey, sub);
    mqtt.publish(topic, payload, true);
    Serial.printf("[MESH→MQTT] %s\n", topic);
}

// Cridat des de wifiMqttTask (via mqtt.loop())
void mqttCallback(char *topic, byte *payload, unsigned int length) {
    MqttTxMsg msg;
    size_t len = min(length, (unsigned int)MSG_MAX_LEN);
    memcpy(msg.payload, payload, len);
    msg.payload[len] = '\0';
    Serial.printf("[MQTT] %s → %s\n", topic, msg.payload);
    xQueueSend(mqttTxQueue, &msg, 0);
    flashSet(GW_COLOR_TX_R);
}

// --- Callbacks mesh (cridats des de meshTask) ---
void onReceive(uint32_t from, String &msg) {
    // Actualitzar registre meshId → nodeId llegible
    StaticJsonDocument<64> idDoc;
    if (deserializeJson(idDoc, msg) == DeserializationError::Ok && idDoc["id"]) {
        nodeRegistry[from] = idDoc["id"].as<String>();
    }

    MeshRxMsg item;
    item.from = from;
    strncpy(item.payload, msg.c_str(), MSG_MAX_LEN);
    item.payload[MSG_MAX_LEN] = '\0';
    xQueueSend(meshRxQueue, &item, 0);
    flashSet(GW_COLOR_RX_R);
}

void onNewConnection(uint32_t nodeId) {
    meshNodeCount++;
    Serial.printf("[MESH] Node connectat:    %u  (xarxa: %u nodes)\n",
                  nodeId, (uint32_t)meshNodeCount + 1);
}

void onDroppedConnection(uint32_t nodeId) {
    if (meshNodeCount > 0) meshNodeCount--;

    // Publicar estat offline usant el nodeId llegible si el coneixem
    String knownId = nodeRegistry.count(nodeId)
                     ? nodeRegistry[nodeId]
                     : String(nodeId);

    StaticJsonDocument<128> doc;
    doc["id"]   = knownId;
    doc["type"] = "status";
    JsonObject data = doc.createNestedObject("data");
    data["status"] = "offline";

    MeshRxMsg item;
    item.from = nodeId;
    serializeJson(doc, item.payload, MSG_MAX_LEN);
    xQueueSend(meshRxQueue, &item, 0);

    nodeRegistry.erase(nodeId);
    Serial.printf("[MESH] Node desconnectat: %u (%s)\n", nodeId, knownId.c_str());
}

// --- meshTask — Core 0 ---
void meshTask(void *param) {
    for (;;) {
        mesh.update();

        // Envia a la mesh les comandes que han arribat per MQTT
        MqttTxMsg txMsg;
        while (xQueueReceive(mqttTxQueue, &txMsg, 0) == pdTRUE) {
            mesh.sendBroadcast(String(txMsg.payload));
            Serial.printf("[MQTT→MESH] broadcast: %s\n", txMsg.payload);
        }

        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

// --- wifiMqttTask — Core 1 ---
void wifiMqttTask(void *param) {
    unsigned long lastWifiAttempt  = 0;
    unsigned long lastMqttAttempt  = 0;
    unsigned long lastBridgeStatus = 0;

    for (;;) {
        // --- WiFi ---
        if (WiFi.status() != WL_CONNECTED) {
            if (millis() - lastWifiAttempt > WIFI_RETRY_MS) {
                lastWifiAttempt = millis();
                Serial.println("[WIFI] Reconnectant...");
                WiFi.begin(WIFI_SSID, WIFI_PASS);
            }
        } else {
            // --- MQTT ---
            if (!mqtt.connected()) {
                if (millis() - lastMqttAttempt > MQTT_RETRY_MS) {
                    lastMqttAttempt = millis();
                    Serial.print("[MQTT] Connectant...");
                    if (mqtt.connect("mesh-bridge")) {
                        Serial.println(" OK");
                        char sub[32];
                        snprintf(sub, sizeof(sub), "%s/+/set", MQTT_PREFIX);
                        mqtt.subscribe(sub);
                    } else {
                        Serial.printf(" Error %d\n", mqtt.state());
                    }
                }
            } else {
                mqtt.loop();

                // Publica a MQTT els missatges bufferitzats de la mesh
                MeshRxMsg rxMsg;
                while (xQueueReceive(meshRxQueue, &rxMsg, 0) == pdTRUE) {
                    mqttPublish(rxMsg.from, rxMsg.payload);
                }
            }
        }

        // Status periòdic del bridge (cada 30s)
        if (mqtt.connected() && millis() - lastBridgeStatus > 30000) {
            lastBridgeStatus = millis();
            StaticJsonDocument<128> doc;
            doc["uptime_s"]  = millis() / 1000;
            doc["mesh_nodes"] = (uint32_t)meshNodeCount;
            doc["free_heap"] = ESP.getFreeHeap();
            char buf[128];
            serializeJson(doc, buf, sizeof(buf));
            char topic[32];
            snprintf(topic, sizeof(topic), "%s/bridge/status", MQTT_PREFIX);
            mqtt.publish(topic, buf, true);
            Serial.printf("[BRIDGE] status: %s\n", buf);
        }

        updateNeo();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// --- Setup ---
void setup() {
    Serial.begin(115200);

    gwNeo.begin();
    gwNeo.clear();
    gwNeo.show();

    // Pas 1: obtenir canal del router (amb timeout per no bloquejar per sempre)
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("[WIFI] Connectant");
    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < WIFI_CONNECT_TIMEOUT) {
        delay(200);
        Serial.print(".");
    }
    uint8_t channel;
    if (WiFi.status() == WL_CONNECTED) {
        channel = WiFi.channel();
        Serial.printf("\n[WIFI] Connectat. Canal: %d\n", channel);
    } else {
        channel = 1;
        Serial.printf("\n[WIFI] Timeout — canal per defecte: %d. "
                      "wifiMqttTask gestionarà la reconnexió.\n", channel);
    }

    // Pas 2: inicialitzar mesh en mode AP al canal del router
    mesh.setDebugMsgTypes(ERROR | STARTUP);
    mesh.init(MESH_SSID, MESH_PASS, &userScheduler, MESH_PORT, WIFI_AP, channel);
    mesh.onReceive(&onReceive);
    mesh.onNewConnection(&onNewConnection);
    mesh.onDroppedConnection(&onDroppedConnection);

    // Pas 3: restaurar STA — la reconnexió la gestiona wifiMqttTask
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt.setCallback(mqttCallback);

    // Crear cues
    meshRxQueue = xQueueCreate(MESH_RX_QUEUE_LEN, sizeof(MeshRxMsg));
    mqttTxQueue = xQueueCreate(MQTT_TX_QUEUE_LEN, sizeof(MqttTxMsg));

    // Crear tasques FreeRTOS
    xTaskCreatePinnedToCore(meshTask,     "mesh",      8192, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(wifiMqttTask, "wifi_mqtt", 8192, NULL, 1, NULL, 1);

    Serial.printf("[INIT] Bridge llest. Node ID: %u\n", mesh.getNodeId());
}

void loop() {
    // Tot corre a les tasques FreeRTOS — loop() queda suspès
    vTaskDelay(portMAX_DELAY);
}

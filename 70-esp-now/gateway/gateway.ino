/*
  gateway.ino — Gateway ESP-NOW ↔ MQTT
  =====================================
  Rol: pont entre la xarxa ESP-NOW i un broker MQTT.
  Board: esp32:esp32:esp32s3  (ESP32-S3 DevKitC-1)
  LED integrat: NeoPixel WS2812 GPIO48 — ambre=heartbeat, verd=ESP-NOW rebut, blau=ESP-NOW enviat

  Configuració obligatòria (#defines a continuació):
    WIFI_SSID / WIFI_PASS  — xarxa WiFi amb accés a internet/broker
    MQTT_SERVER            — IP o hostname del broker MQTT
    MQTT_PORT              — port del broker (per defecte 1883)

  Topics MQTT generats:
    espnow/{node_id}/state  ← publica quan rep sensor/event/state
    espnow/{node_id}/set    → subscriu per enviar comandes als nodes (pendent)

  Registre de nodes (LittleFS /nodes.json):
    Quan un node envia "announce", la gateway desa node_id ↔ MAC + caps.
    Persisteix entre reinicis. Necessari per saber a quina MAC enviar comandes.

  Decisions de disseny:
    - WIFI_STA (no AP_STA): simplifica la gestió de canals i MACs.
    - esp_wifi_set_ps(WIFI_PS_NONE): el power save del driver adorm el ràdio
      i causa pèrdua de paquets ESP-NOW dels nodes. Cal desactivar-lo explícitament.
    - No cal registrar els nodes com a peers per rebre'ls; el callback
      esp_now_register_recv_cb dispara per qualsevol paquet entrant.

  Compilar i flashejar:
    arduino-cli compile --fqbn esp32:esp32:esp32s3 gateway/
    arduino-cli upload  --fqbn esp32:esp32:esp32s3 --port /dev/ttyACM0 gateway/

  Llibreries necessàries:
    ArduinoJson >= 7.x   (arduino-cli lib install "ArduinoJson")
    PubSubClient 2.8     (arduino-cli lib install "PubSubClient")
*/

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include "message.h"

// --- Configuració ---
#define WIFI_SSID     "la-teva-xarxa"
#define WIFI_PASS     "la-teva-contrasenya"
#define MQTT_SERVER   "ip-del-broker"
#define MQTT_PORT     1883
#define MQTT_PREFIX   "espnow"       // topics: espnow/{id}/state, espnow/{id}/set

#define REGISTRY_FILE "/nodes.json"  // fitxer de registre a LittleFS

// LED d'estat integrat (NeoPixel WS2812 a GPIO48 del DevKitC-1)
#define GW_NEO_PIN            48
#define GW_FLASH_MS          150   // durada flash d'activitat
#define GW_HEARTBEAT_MS     1000   // interval entre polsos
#define GW_HEARTBEAT_PULSE    80   // durada del pols de heartbeat

// Colors: verd = rebut ESP-NOW, blau = enviat ESP-NOW, ambre = heartbeat
#define GW_COLOR_RX_R   0,  150,  0
#define GW_COLOR_TX_R   0,    0, 150
#define GW_COLOR_HB_R  15,    8,  0

// --- Globals ---
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

Adafruit_NeoPixel gwNeo(1, GW_NEO_PIN, NEO_GRB + NEO_KHZ800);

// Flash d'activitat: escrit des del WiFi task (volatile), llegit des de loop()
volatile bool    flashPending = false;
volatile uint8_t pendingR = 0, pendingG = 0, pendingB = 0;
unsigned long    flashUntil = 0;

// Heartbeat
unsigned long heartbeatNext  = 0;
unsigned long heartbeatPulseUntil = 0;

void neoOff() {
    gwNeo.clear();
    gwNeo.show();
}

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

    // Flash d'activitat té prioritat
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
        flashUntil = 0;
        neoOff();
        return;
    }

    // Heartbeat
    if (heartbeatPulseUntil > 0) {
        if (now >= heartbeatPulseUntil) {
            heartbeatPulseUntil = 0;
            neoOff();
        }
        return;
    }

    if (now >= heartbeatNext) {
        heartbeatNext = now + GW_HEARTBEAT_MS;
        heartbeatPulseUntil = now + GW_HEARTBEAT_PULSE;
        neoSet(GW_COLOR_HB_R);
    }
}

// --- Registre de nodes ---
void registryLoad(JsonDocument &doc) {
    if (!LittleFS.exists(REGISTRY_FILE)) return;
    File f = LittleFS.open(REGISTRY_FILE, "r");
    deserializeJson(doc, f);
    f.close();
}

void registrySave(JsonDocument &doc) {
    File f = LittleFS.open(REGISTRY_FILE, "w");
    serializeJson(doc, f);
    f.close();
}

void registryUpdate(const char *nodeId, const uint8_t *mac, JsonArray caps) {
    StaticJsonDocument<2048> registry;
    registryLoad(registry);

    JsonObject entry = registry[nodeId].isNull()
        ? registry.createNestedObject(nodeId)
        : registry[nodeId].as<JsonObject>();

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    entry["mac"]       = macStr;
    entry["last_seen"] = millis() / 1000;
    // Reescriure caps (pot canviar entre reinicis del node)
    entry.remove("caps");
    JsonArray entryCaps = entry.createNestedArray("caps");
    for (const char *cap : caps) entryCaps.add(cap);

    registrySave(registry);
    Serial.printf("[REGISTRY] Node registrat: %s (%s)\n", nodeId, macStr);
}

// Retorna la MAC d'un node registrat, o false si no existeix
bool registryGetMac(const char *nodeId, uint8_t *macOut) {
    StaticJsonDocument<2048> registry;
    registryLoad(registry);
    if (registry[nodeId].isNull()) return false;
    const char *macStr = registry[nodeId]["mac"];
    if (!macStr) return false;
    sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &macOut[0], &macOut[1], &macOut[2], &macOut[3], &macOut[4], &macOut[5]);
    return true;
}

// --- ESP-NOW send callback ---
void onSent(const esp_now_send_info_t *info, esp_now_send_status_t status) {
    Serial.printf("[ESP-NOW] Comanda enviada a %02X:%02X:%02X:%02X:%02X:%02X: %s\n",
                  info->des_addr[0], info->des_addr[1], info->des_addr[2],
                  info->des_addr[3], info->des_addr[4], info->des_addr[5],
                  status == ESP_NOW_SEND_SUCCESS ? "OK" : "ERROR");
    if (status == ESP_NOW_SEND_SUCCESS) flashSet(GW_COLOR_TX_R);
}

// --- MQTT ---
void mqttPublish(const char *nodeId, const char *subTopic, JsonDocument &payload) {
    char topic[64];
    snprintf(topic, sizeof(topic), "%s/%s/%s", MQTT_PREFIX, nodeId, subTopic);
    char buf[ESPNOW_MAX_PAYLOAD];
    serializeJson(payload, buf, sizeof(buf));
    mqtt.publish(topic, buf, true);  // retain=true per tenir l'últim estat disponible
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
    // Topic format: espnow/{nodeId}/set
    char buf[ESPNOW_MAX_PAYLOAD + 1];
    memcpy(buf, payload, length);
    buf[length] = '\0';
    Serial.printf("[MQTT] %s → %s\n", topic, buf);

    // Extreure nodeId del topic: "espnow/{nodeId}/set"
    char nodeId[32] = {};
    const char *start = topic + strlen(MQTT_PREFIX) + 1;
    const char *end   = strrchr(topic, '/');
    if (!end || end <= start) return;
    size_t idLen = end - start;
    if (idLen == 0 || idLen >= sizeof(nodeId)) return;
    memcpy(nodeId, start, idLen);

    // Buscar la MAC del node al registre
    uint8_t mac[6];
    if (!registryGetMac(nodeId, mac)) {
        Serial.printf("[MQTT→ESP-NOW] Node '%s' no trobat al registre\n", nodeId);
        return;
    }

    // Validar que el payload és JSON vàlid (serà el camp "data" de la comanda)
    StaticJsonDocument<ESPNOW_MAX_PAYLOAD> dataDoc;
    if (deserializeJson(dataDoc, buf) != DeserializationError::Ok) {
        Serial.println("[MQTT→ESP-NOW] Payload JSON invàlid");
        return;
    }

    // Construir missatge command
    StaticJsonDocument<ESPNOW_MAX_PAYLOAD> cmd;
    cmd["id"]   = nodeId;
    cmd["type"] = MSG_COMMAND;
    cmd["data"] = dataDoc.as<JsonVariant>();

    // Afegir el node com a peer ESP-NOW si no ho és ja
    if (!esp_now_is_peer_exist(mac)) {
        esp_now_peer_info_t peer = {};
        memcpy(peer.peer_addr, mac, 6);
        peer.channel = 0;       // 0 = usar el canal WiFi actual
        peer.encrypt = false;
        esp_now_add_peer(&peer);
    }

    // Enviar via ESP-NOW
    char cmdBuf[ESPNOW_MAX_PAYLOAD];
    size_t cmdLen = serializeJson(cmd, cmdBuf, sizeof(cmdBuf));
    esp_err_t result = esp_now_send(mac, (uint8_t *)cmdBuf, cmdLen);
    Serial.printf("[MQTT→ESP-NOW] %s → %s [%s]\n", nodeId, cmdBuf,
                  result == ESP_OK ? "enviat" : "error send");
}

void mqttConnect() {
    while (!mqtt.connected()) {
        Serial.print("[MQTT] Connectant...");
        if (mqtt.connect("esp-now-gateway")) {
            Serial.println(" OK");
            char subTopic[32];
            snprintf(subTopic, sizeof(subTopic), "%s/+/set", MQTT_PREFIX);
            mqtt.subscribe(subTopic);
        } else {
            Serial.printf(" Error %d, reintentant en 5s\n", mqtt.state());
            delay(5000);
        }
    }
}

// --- ESP-NOW callback ---
void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    char buf[ESPNOW_MAX_PAYLOAD + 1];
    memcpy(buf, data, len);
    buf[len] = '\0';
    Serial.printf("[ESP-NOW] Rebut: %s\n", buf);

    StaticJsonDocument<ESPNOW_MAX_PAYLOAD> doc;
    if (deserializeJson(doc, buf) != DeserializationError::Ok) {
        Serial.println("[ERROR] JSON invàlid");
        return;
    }

    const char *nodeId = doc["id"];
    const char *type   = doc["type"];
    if (!nodeId || !type) return;

    flashSet(GW_COLOR_RX_R);

    if (strcmp(type, MSG_ANNOUNCE) == 0) {
        registryUpdate(nodeId, info->src_addr, doc["caps"].as<JsonArray>());

    } else if (strcmp(type, MSG_SENSOR) == 0) {
        mqttPublish(nodeId, "state", doc);

    } else if (strcmp(type, MSG_EVENT) == 0) {
        mqttPublish(nodeId, "event", doc);

    } else if (strcmp(type, MSG_ACK) == 0) {
        Serial.printf("[ACK] %s ha confirmat comanda\n", nodeId);
    }
}

// --- Setup / Loop ---
void setup() {
    Serial.begin(115200);

    if (!LittleFS.begin(true)) {
        Serial.println("[ERROR] LittleFS init fallida");
        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("[WIFI] Connectant");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    // Power save desactivat: amb PS actiu el ràdio dorm entre balises de l'AP
    // i el driver descarta els paquets ESP-NOW dels nodes durant aquells intervals
    esp_wifi_set_ps(WIFI_PS_NONE);
    Serial.printf("\n[WIFI] Connectat. IP: %s  Canal: %d  MAC: %s\n",
                  WiFi.localIP().toString().c_str(), WiFi.channel(),
                  WiFi.macAddress().c_str());

    gwNeo.begin();
    gwNeo.clear();
    gwNeo.show();

    if (esp_now_init() != ESP_OK) {
        Serial.println("[ERROR] ESP-NOW init fallida");
        return;
    }
    esp_now_register_recv_cb(onReceive);
    esp_now_register_send_cb(onSent);

    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt.setCallback(mqttCallback);
    mqttConnect();

    Serial.println("[INIT] Gateway llesta");
}

void loop() {
    if (!mqtt.connected()) mqttConnect();
    mqtt.loop();
    updateNeo();
}

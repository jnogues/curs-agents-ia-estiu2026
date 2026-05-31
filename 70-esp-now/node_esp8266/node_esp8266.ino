/*
  node_esp8266.ino — Node ESP-NOW per a NodeMCU ESP8266
  ======================================================
  Rol: envia dades de sensors/events a la gateway i controla actuadors.
  Board: esp8266:esp8266:nodemcuv2

  Sensors:
    LM75B (I2C)   SDA→D2(GPIO4), SCL→D1(GPIO5), adreça 0x48 (A0=A1=A2=GND)
    Potenciòmetre A0 (0–1023, 10 bits)

  Actuadors:
    LED positiu   D7  (GPIO13) — HIGH=ON,  LOW=OFF
    LED negatiu   D3  (GPIO0)  — LOW=ON,  HIGH=OFF  (lògica negativa)
    LED negatiu   D4  (GPIO2)  — LOW=ON,  HIGH=OFF  (lògica negativa)
    NeoPixel      D8  (GPIO15) — 1 píxel WS2812
    LED integrat  D0  (GPIO16) — LOW=ON · heartbeat 100ms (indicador de vida)

  Entrades:
    Polsador      D6  (GPIO12) — pull-down extern, HIGH quan premut
    Polsador      D5  (GPIO14) — pull-down extern, HIGH quan premut

  Notes de boot ESP8266:
    GPIO0 i GPIO2 han de ser HIGH en arrencar (strapping pins).
    Amb lògica negativa el LED és naturalment apagat (HIGH) durant el boot.
    GPIO15 ha de ser LOW en arrencar; el NeoPixel no reacciona a nivell baix.

  Configuració obligatòria:
    NODE_NAME       — nom base del node
    GATEWAY_CHANNEL — canal WiFi del router de la gateway
    GATEWAY_MAC     — MAC de la gateway (impresa al serial en arrencar)

  Compilar i flashejar:
    arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 node_esp8266/
    arduino-cli upload  --fqbn esp8266:esp8266:nodemcuv2 --port /dev/ttyUSB0 node_esp8266/

  Comandes MQTT (topic espnow/{id}/set):
    LED GPIO13:   {"target":"led13", "state":"on"}  / {"target":"led13", "state":"off"}
    LED GPIO0:    {"target":"led0",  "state":"on"}  / {"target":"led0",  "state":"off"}
    LED GPIO2:    {"target":"led2",  "state":"on"}  / {"target":"led2",  "state":"off"}
    NeoPixel:     {"target":"neo", "r":255, "g":0, "b":128}
                  {"target":"neo", "state":"off"}

  Events MQTT publicats (topic espnow/{id}/event):
    Polsador:     {"trigger":"btn12", "state":"pressed"}
                  {"trigger":"btn14", "state":"pressed"}
*/

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include "message.h"
extern "C" {
  #include "user_interface.h"   // wifi_set_channel / wifi_get_channel
}

// --- Configuració ---
#define NODE_NAME       "node1"
#define SEND_INTERVAL   10000       // ms entre enviaments de sensor
#define GATEWAY_CHANNEL 1
#define GATEWAY_MAC     {0x80, 0xB5, 0x4E, 0xC4, 0xED, 0x3C}

// Pins
#define LM75B_ADDR   0x48
#define LM75B_SDA    4    // D2
#define LM75B_SCL    5    // D1
#define LED_POS      13   // D7 — lògica positiva
#define LED_NEG0     0    // D3 — lògica negativa (boot pin, ha de ser HIGH en arrencar)
#define LED_NEG2     2    // D4 — lògica negativa (boot pin, ha de ser HIGH en arrencar)
#define BTN_12       12   // D6 — pull-down extern
#define BTN_14       14   // D5 — pull-down extern
#define NEO_PIN      15   // D8
#define NEO_COUNT    1
#define HEARTBEAT_PIN  16   // D0 — LED integrat NodeMCU, lògica negativa (LOW=encès)

#define BTN_DEBOUNCE   50   // ms
#define HEARTBEAT_MS  100   // interval toggle heartbeat

// --- Globals ---
char nodeId[32];
uint8_t gatewayMac[]   = GATEWAY_MAC;
uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

Adafruit_NeoPixel neo(NEO_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

bool btnState12 = false;
bool btnState14 = false;
unsigned long btnDebounce12 = 0;
unsigned long btnDebounce14 = 0;

// --- LM75B ---
float lm75bRead() {
    Wire.beginTransmission(LM75B_ADDR);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) return NAN;
    Wire.requestFrom(LM75B_ADDR, 2);
    if (Wire.available() < 2) return NAN;
    int16_t raw = (Wire.read() << 8) | Wire.read();
    return (raw >> 5) * 0.125f;
}

// --- Actuadors ---
void setLed(uint8_t pin, bool negativeLogic, bool on) {
    digitalWrite(pin, negativeLogic ? !on : on);
}

void setNeo(uint8_t r, uint8_t g, uint8_t b) {
    neo.setPixelColor(0, neo.Color(r, g, b));
    neo.show();
}

// --- Callbacks ESP-NOW ---
void onSent(uint8_t *mac, uint8_t status) {
    Serial.printf("[ESP-NOW] Enviat: %s\n", status == 0 ? "OK" : "ERROR");
}

void onReceive(uint8_t *mac, uint8_t *data, uint8_t len) {
    char buf[ESPNOW_MAX_PAYLOAD + 1];
    memcpy(buf, data, len);
    buf[len] = '\0';
    Serial.printf("[ESP-NOW] Rebut: %s\n", buf);

    StaticJsonDocument<ESPNOW_MAX_PAYLOAD> doc;
    if (deserializeJson(doc, buf) != DeserializationError::Ok) return;

    const char *id = doc["id"];
    if (!id || strcmp(id, nodeId) != 0) return;

    const char *type = doc["type"];
    if (strcmp(type, MSG_COMMAND) != 0) return;

    const char *target = doc["data"]["target"];
    const char *state  = doc["data"]["state"];

    if (!target) {
        Serial.println("[CMD] camp 'target' absent");
        sendAck();
        return;
    }

    if (strcmp(target, "led13") == 0 && state) {
        bool on = strcmp(state, "on") == 0;
        setLed(LED_POS, false, on);
        Serial.printf("[CMD] LED13 %s\n", on ? "ON" : "OFF");

    } else if (strcmp(target, "led0") == 0 && state) {
        bool on = strcmp(state, "on") == 0;
        setLed(LED_NEG0, true, on);
        Serial.printf("[CMD] LED0 %s\n", on ? "ON" : "OFF");

    } else if (strcmp(target, "led2") == 0 && state) {
        bool on = strcmp(state, "on") == 0;
        setLed(LED_NEG2, true, on);
        Serial.printf("[CMD] LED2 %s\n", on ? "ON" : "OFF");

    } else if (strcmp(target, "neo") == 0) {
        if (state && strcmp(state, "off") == 0) {
            setNeo(0, 0, 0);
            Serial.println("[CMD] NEO OFF");
        } else {
            uint8_t r = doc["data"]["r"] | 0;
            uint8_t g = doc["data"]["g"] | 0;
            uint8_t b = doc["data"]["b"] | 0;
            setNeo(r, g, b);
            Serial.printf("[CMD] NEO RGB(%d,%d,%d)\n", r, g, b);
        }
    } else {
        Serial.printf("[CMD] target desconegut: %s\n", target);
    }

    sendAck();
}

// --- Funcions d'enviament ---
void sendJson(JsonDocument &doc) {
    char buf[ESPNOW_MAX_PAYLOAD];
    size_t len = serializeJson(doc, buf, sizeof(buf));
    esp_now_send(broadcastMac, (uint8_t *)buf, len);
}

void sendAnnounce() {
    StaticJsonDocument<ESPNOW_MAX_PAYLOAD> doc;
    doc["id"]   = nodeId;
    doc["type"] = MSG_ANNOUNCE;
    JsonArray caps = doc.createNestedArray("caps");
    caps.add(CAP_SENSOR);
    caps.add(CAP_COMMAND);
    caps.add(CAP_EVENT);
    sendJson(doc);
    Serial.println("[ANNOUNCE] enviat");
}

void sendSensor() {
    float temp = lm75bRead();
    if (isnan(temp)) {
        Serial.println("[LM75B] Error de lectura I2C");
        return;
    }
    int pot = analogRead(A0);
    Serial.printf("[SENSOR] temp=%.3f°C  pot=%d\n", temp, pot);

    StaticJsonDocument<ESPNOW_MAX_PAYLOAD> doc;
    doc["id"]   = nodeId;
    doc["type"] = MSG_SENSOR;
    JsonObject data = doc.createNestedObject("data");
    data["temp"] = serialized(String(temp, 2));
    data["pot"]  = pot;
    sendJson(doc);
}

void sendEvent(const char *trigger) {
    StaticJsonDocument<ESPNOW_MAX_PAYLOAD> doc;
    doc["id"]   = nodeId;
    doc["type"] = MSG_EVENT;
    JsonObject data = doc.createNestedObject("data");
    data["trigger"] = trigger;
    data["state"]   = "pressed";
    sendJson(doc);
    Serial.printf("[EVENT] %s\n", trigger);
}

void sendAck() {
    StaticJsonDocument<ESPNOW_MAX_PAYLOAD> doc;
    doc["id"]   = nodeId;
    doc["type"] = MSG_ACK;
    doc["cmd"]  = MSG_COMMAND;
    sendJson(doc);
    Serial.println("[ACK] enviat");
}

// --- Setup / Loop ---
void setup() {
    Serial.begin(115200);

    // LEDs
    pinMode(LED_POS,       OUTPUT); digitalWrite(LED_POS,       LOW);   // apagat
    pinMode(LED_NEG0,      OUTPUT); digitalWrite(LED_NEG0,      HIGH);  // apagat (lògica negativa)
    pinMode(LED_NEG2,      OUTPUT); digitalWrite(LED_NEG2,      HIGH);  // apagat (lògica negativa)
    pinMode(HEARTBEAT_PIN, OUTPUT); digitalWrite(HEARTBEAT_PIN, HIGH);  // apagat (lògica negativa)

    // Polsadors
    pinMode(BTN_12, INPUT);
    pinMode(BTN_14, INPUT);

    // NeoPixel
    neo.begin();
    neo.clear();
    neo.show();

    // LM75B
    Wire.begin(LM75B_SDA, LM75B_SCL);
    Wire.beginTransmission(LM75B_ADDR);
    Serial.printf("[LM75B] %s a 0x%02X\n",
                  Wire.endTransmission() == 0 ? "Detectat" : "ADVERTIMENT: no detectat",
                  LM75B_ADDR);

    // Node ID
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(nodeId, sizeof(nodeId), "%s_%02X%02X%02X", NODE_NAME, mac[3], mac[4], mac[5]);
    Serial.printf("[INIT] Node ID: %s  MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  nodeId, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // ESP-NOW
    if (esp_now_init() != 0) {
        Serial.println("[ERROR] ESP-NOW init fallida");
        return;
    }
    wifi_set_channel(GATEWAY_CHANNEL);
    Serial.printf("[INIT] Canal: %d\n", wifi_get_channel());

    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_add_peer(gatewayMac,   ESP_NOW_ROLE_COMBO, GATEWAY_CHANNEL, NULL, 0);
    esp_now_add_peer(broadcastMac, ESP_NOW_ROLE_COMBO, GATEWAY_CHANNEL, NULL, 0);
    esp_now_register_send_cb(onSent);
    esp_now_register_recv_cb(onReceive);

    sendAnnounce();
}

void loop() {
    // Heartbeat GPIO16
    static unsigned long lastHeartbeat = 0;
    static bool heartbeatState = false;
    if (millis() - lastHeartbeat >= HEARTBEAT_MS) {
        lastHeartbeat = millis();
        heartbeatState = !heartbeatState;
        digitalWrite(HEARTBEAT_PIN, heartbeatState ? LOW : HIGH);
    }

    // Sensor periòdic
    static unsigned long lastSend = 0;
    if (millis() - lastSend >= SEND_INTERVAL) {
        lastSend = millis();
        sendSensor();
    }

    // Polsador GPIO12
    bool btn12Now = digitalRead(BTN_12);
    if (btn12Now && !btnState12 && millis() - btnDebounce12 > BTN_DEBOUNCE) {
        btnDebounce12 = millis();
        btnState12 = true;
        sendEvent("btn12");
    } else if (!btn12Now) {
        btnState12 = false;
    }

    // Polsador GPIO14
    bool btn14Now = digitalRead(BTN_14);
    if (btn14Now && !btnState14 && millis() - btnDebounce14 > BTN_DEBOUNCE) {
        btnDebounce14 = millis();
        btnState14 = true;
        sendEvent("btn14");
    } else if (!btn14Now) {
        btnState14 = false;
    }
}

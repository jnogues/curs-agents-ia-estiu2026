/*
  mesh_node.ino — Node mesh amb sensors i actuadors
  ==================================================
  Rol: node de la xarxa painlessMesh. Envia dades de sensors periòdicament,
  publica events de polsadors i controla actuadors per comandes.
  Board: esp8266:esp8266:nodemcuv2

  Sensors:
    LM75B (I2C)    SDA→D2(GPIO4), SCL→D1(GPIO5), adreça 0x48
    Potenciòmetre  A0 (0–1023)

  Actuadors:
    LED positiu    D7  (GPIO13) — HIGH=ON
    LED negatiu    D3  (GPIO0)  — LOW=ON  (boot pin)
    LED negatiu    D4  (GPIO2)  — LOW=ON  (boot pin)
    NeoPixel       D8  (GPIO15) — 1 píxel WS2812
    LED integrat   D0  (GPIO16) — LOW=ON · heartbeat 100ms

  Entrades:
    Polsador 1     D6  (GPIO12) — pull-down extern
    Polsador 2     D5  (GPIO14) — pull-down extern

  Compilar i flashejar:
    arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 mesh_node/
    arduino-cli upload  --fqbn esp8266:esp8266:nodemcuv2 --port /dev/ttyUSB0 mesh_node/

  Format de missatges (JSON, broadcast a la mesh):
    Sensor:  {"id":"node1_XXYYZZ","type":"sensor","data":{"temp":25.50,"pot":512}}
    Event:   {"id":"node1_XXYYZZ","type":"event","data":{"trigger":"btn12","state":"pressed"}}
    Comanda: {"id":"node1_XXYYZZ","type":"command","data":{"target":"led13","state":"on"}}
             {"id":"node1_XXYYZZ","type":"command","data":{"target":"neo","r":255,"g":0,"b":0}}
*/

#include <ESP8266WiFi.h>
#include <painlessMesh.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

// --- Configuració ---
#define NODE_NAME      "node1"
#define SEND_INTERVAL  10000     // ms entre enviaments de sensor

#define MESH_SSID      "mesh-lab"
#define MESH_PASS      "meshpass123"
#define MESH_PORT      5555

// Pins
#define LM75B_ADDR    0x48
#define LM75B_SDA     4     // D2
#define LM75B_SCL     5     // D1
#define LED_POS       13    // D7 — lògica positiva
#define LED_NEG0      0     // D3 — lògica negativa (boot pin)
#define LED_NEG2      2     // D4 — lògica negativa (boot pin)
#define BTN_12        12    // D6 — pull-down extern
#define BTN_14        14    // D5 — pull-down extern
#define NEO_PIN       15    // D8
#define NEO_COUNT     1
#define HEARTBEAT_PIN 16    // D0 — LED integrat NodeMCU, lògica negativa

#define BTN_DEBOUNCE  50    // ms
#define HEARTBEAT_MS 100    // ms entre toggles

// --- Globals ---
Scheduler userScheduler;
painlessMesh mesh;
Adafruit_NeoPixel neo(NEO_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

char nodeId[32];
bool btnState12 = false, btnState14 = false;
unsigned long btnDebounce12 = 0, btnDebounce14 = 0;

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
    neo.show();  // doble show: WiFi ISR pot interrompre el primer en ESP8266
}

// --- Enviament ---
void sendJson(JsonDocument &doc) {
    char buf[250];
    serializeJson(doc, buf, sizeof(buf));
    mesh.sendBroadcast(String(buf));
}

void sendSensor() {
    float temp = lm75bRead();
    if (isnan(temp)) { Serial.println("[LM75B] Error de lectura"); return; }
    int pot = analogRead(A0);
    Serial.printf("[SENSOR] temp=%.2f°C  pot=%d\n", temp, pot);

    StaticJsonDocument<250> doc;
    doc["id"]   = nodeId;
    doc["type"] = "sensor";
    doc["ts"]   = mesh.getNodeTime() / 1000;  // µs → ms (sinc. a tota la mesh)
    JsonObject data = doc.createNestedObject("data");
    data["temp"] = serialized(String(temp, 2));
    data["pot"]  = pot;
    sendJson(doc);
}

void sendEvent(const char *trigger) {
    StaticJsonDocument<250> doc;
    doc["id"]   = nodeId;
    doc["type"] = "event";
    doc["ts"]   = mesh.getNodeTime() / 1000;
    JsonObject data = doc.createNestedObject("data");
    data["trigger"] = trigger;
    data["state"]   = "pressed";
    sendJson(doc);
    Serial.printf("[EVENT] %s\n", trigger);
}

void sendAck(const char *cmd) {
    StaticJsonDocument<128> doc;
    doc["id"]   = nodeId;
    doc["type"] = "ack";
    doc["ts"]   = mesh.getNodeTime() / 1000;
    doc["cmd"]  = cmd;
    sendJson(doc);
    Serial.printf("[ACK] %s\n", cmd);
}

// --- Callback recepció ---
void onReceive(uint32_t from, String &msg) {
    Serial.printf("[RECV] %u: %s\n", from, msg.c_str());

    StaticJsonDocument<250> doc;
    if (deserializeJson(doc, msg) != DeserializationError::Ok) return;

    const char *id   = doc["id"];
    const char *type = doc["type"];
    if (!id || strcmp(id, nodeId) != 0) return;
    if (!type || strcmp(type, "command") != 0) return;

    const char *target = doc["data"]["target"];
    const char *state  = doc["data"]["state"];
    if (!target) return;

    if (strcmp(target, "led13") == 0 && state) {
        setLed(LED_POS, false, strcmp(state, "on") == 0);
        Serial.printf("[CMD] LED13 %s\n", state);

    } else if (strcmp(target, "led0") == 0 && state) {
        setLed(LED_NEG0, true, strcmp(state, "on") == 0);
        Serial.printf("[CMD] LED0 %s\n", state);

    } else if (strcmp(target, "led2") == 0 && state) {
        setLed(LED_NEG2, true, strcmp(state, "on") == 0);
        Serial.printf("[CMD] LED2 %s\n", state);

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
        return;  // target desconegut, no enviar ACK
    }

    sendAck(target);
}

Task taskSensor(SEND_INTERVAL, TASK_FOREVER, &sendSensor);

// --- Setup / Loop ---
void setup() {
    Serial.begin(115200);

    // LEDs
    pinMode(LED_POS,       OUTPUT); digitalWrite(LED_POS,       LOW);
    pinMode(LED_NEG0,      OUTPUT); digitalWrite(LED_NEG0,      HIGH);
    pinMode(LED_NEG2,      OUTPUT); digitalWrite(LED_NEG2,      HIGH);
    pinMode(HEARTBEAT_PIN, OUTPUT); digitalWrite(HEARTBEAT_PIN, HIGH);

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
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(nodeId, sizeof(nodeId), "%s_%02X%02X%02X", NODE_NAME, mac[3], mac[4], mac[5]);

    // painlessMesh
    mesh.setDebugMsgTypes(ERROR | STARTUP);
    mesh.init(MESH_SSID, MESH_PASS, &userScheduler, MESH_PORT);
    mesh.onReceive(&onReceive);
    mesh.onNewConnection([](uint32_t nodeId) {
        Serial.printf("[MESH] Connectat a: %u\n", nodeId);
    });
    mesh.onDroppedConnection([](uint32_t nodeId) {
        Serial.printf("[MESH] Desconnectat de: %u\n", nodeId);
    });

    userScheduler.addTask(taskSensor);
    taskSensor.enable();

    Serial.printf("[INIT] Node ID: %s  Mesh ID: %u\n", nodeId, mesh.getNodeId());
}

void loop() {
    mesh.update();

    // Heartbeat GPIO16
    static unsigned long lastHeartbeat = 0;
    static bool heartbeatState = false;
    if (millis() - lastHeartbeat >= HEARTBEAT_MS) {
        lastHeartbeat = millis();
        heartbeatState = !heartbeatState;
        digitalWrite(HEARTBEAT_PIN, heartbeatState ? LOW : HIGH);
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

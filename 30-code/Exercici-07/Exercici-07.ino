/*
 * esp32s3_template.ino
 *
 * v7 — Template ESP32-S3 per a Arduino IDE
 * Funcionalitats: WiFi multi-xarxa · MQTT · Neopixel · Temperatura interna + DS18B20 · NTP · LWT · Polsadors + actuadors
 * Arquitectura:   5 tasques FreeRTOS (loop() queda buit)
 *
 * Llibreries necessàries (Arduino Library Manager):
 *   - "MycilaMQTT"          per mathieucarbou
 *   - "Adafruit NeoPixel"   per Adafruit
 *   - "OneWire"             per Jim Studt
 *   - "DallasTemperature"   per Miles Burton
 *   - "ArduinoJson"         per Benoit Blanchon
 *
 * Board: "ESP32S3 Dev Module" (o la teva variant d'ESP32-S3)
 */

// ============================================================
//  CONFIGURACIÓ — modifica aquí els teus paràmetres
// ============================================================

// WiFi — pots configurar fins a 3 xarxes
#define WIFI_SSID_1  "SSID_ALUMNE"
#define WIFI_PASS_1  "PASSWORD_ALUMNE"
#define WIFI_SSID_2  ""                   // deixa buit si no en cal
#define WIFI_PASS_2  ""
#define WIFI_SSID_3  ""                   // deixa buit si no en cal
#define WIFI_PASS_3  ""

// MQTT
#define MQTT_BROKER     "46.224.116.35"
#define MQTT_PORT       1883
#define MQTT_CLIENT_ID  "ESP32S3"   // s'hi afegirà la MAC automàticament

#define ALUMNE_ID       "alumneXX"     // <-- cada alumne canvia aquest valor

#define TOPIC_BASE      "/esp32s3/" ALUMNE_ID
#define TOPIC_ESTAT     TOPIC_BASE "/estat"
#define TOPIC_ORDRE     TOPIC_BASE "/ordre"
#define TOPIC_TEMP_INT  TOPIC_BASE "/temperaturaINT"
#define TOPIC_TEMP_DS   TOPIC_BASE "/temperaturaDS"
#define TOPIC_LEDS      TOPIC_BASE "/stateLEDS"
#define TOPIC_ONLINE    TOPIC_BASE "/online"
#define TOPIC_POLSADORS TOPIC_BASE "/polsadors"

// Hardware
#define NEOPIXEL_PIN    48   // GPIO del LED WS2812B integrat
#define NEOPIXEL_COUNT  1
#define PIN_DS18B20     16
// LEDs externs (actius per HIGH). Cada LED es controla per MQTT:
//   led1 (GPIO7)   led2 (GPIO15)   led3 (GPIO21)   led4 (GPIO47)
// Comandes: "led1 ON", "led1 OFF", "led1 TOG" (idem led2..4)
#define PIN_LED1         7
#define PIN_LED2        15
#define PIN_LED3        21
#define PIN_LED4        47
// Actuadors
#define PIN_TRIAC        6
#define PIN_RELE        18
#define PIN_BUZZER      38
// Polsadors (entrada amb pull-up intern)
#define PIN_POLSADOR1   39
#define PIN_POLSADOR2   40
#define PIN_POLSADOR3   41
#define PIN_POLSADOR4   42

// Precisió del DS18B20 (9..12 bits). 12 bits = 0.0625 °C, 750 ms de conversió.
#define DS18B20_RESOLUTION  12

// Intervals de publicació MQTT (mil·lisegons)
#define INTERVAL_ESTAT  5000
#define INTERVAL_TEMP   10000

// NTP
#define NTP_SERVER      "pool.ntp.org"
#define GMT_OFFSET_SEC  3600
#define DST_OFFSET_SEC  3600

// Debounce polsadors (ms)
#define DEBOUNCE_MS     50

// ============================================================
//  GUIA RÀPIDA — Comandes MQTT (broker 46.224.116.35:1883)
// ============================================================
//
// Subscriure's a tots els tòpics de l'alumne:
//   mosquitto_sub -h 46.224.116.35 -t "/esp32s3/alumneXX/#" -v
//
// Comandes LEDs (topic: /esp32s3/alumneXX/ordre):
//   mosquitto_pub -h 46.224.116.35 -t "/esp32s3/alumneXX/ordre" -m "led1 ON"
//   mosquitto_pub -h 46.224.116.35 -t "/esp32s3/alumneXX/ordre" -m "led2 OFF"
//   mosquitto_pub -h 46.224.116.35 -t "/esp32s3/alumneXX/ordre" -m "led3 TOG"
//   mosquitto_pub -h 46.224.116.35 -t "/esp32s3/alumneXX/ordre" -m "led4 TOG"
//
// Comandes actuadors (topic: /esp32s3/alumneXX/ordre):
//   mosquitto_pub -h 46.224.116.35 -t "/esp32s3/alumneXX/ordre" -m "triac ON"
//   mosquitto_pub -h 46.224.116.35 -t "/esp32s3/alumneXX/ordre" -m "rele OFF"
//   mosquitto_pub -h 46.224.116.35 -t "/esp32s3/alumneXX/ordre" -m "buzzer 2000 30"
//
// Llegir estat retained dels LEDs + actuadors:
//   mosquitto_sub -h 46.224.116.35 -t "/esp32s3/alumneXX/stateLEDS" -C 1
//
// Tòpics de telemetria (només lectura):
//   /esp32s3/alumneXX/estat          → cada 5s  {"time":...,"ip":...,"rssi":...,"uptime":...,"leds":[...],"triac":...,"rele":...,"buzzer":...}
//   /esp32s3/alumneXX/temperaturaINT → cada 10s
//   /esp32s3/alumneXX/temperaturaDS  → cada 10s
//   /esp32s3/alumneXX/online         → "1" si en línia, "0" (LWT) si cau
//   /esp32s3/alumneXX/polsadors      → {"boto":1,"event":"PREMUT"} en prémer / {"boto":1,"event":"ALLIBERAT"} en deixar anar
// ============================================================

#include <WiFi.h>
#include <WiFiMulti.h>
#include <Adafruit_NeoPixel.h>
#include <MycilaMQTT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "driver/temperature_sensor.h"
#include <ArduinoJson.h>
#include "time.h"

// ---- Codificació d'estats del sistema ----
//
//  ESTAT_INIT                → LED blanc fix
//  ESTAT_WIFI_CONNECTANT     → LED blau, parpelleig lent  (1 Hz)
//  ESTAT_WIFI_OK_MQTT_CONN   → LED groc, parpelleig ràpid (4 Hz)
//  ESTAT_TOT_OK              → LED verd, batec lent (1 Hz)
//  ESTAT_WIFI_OK_MQTT_PERDUT → LED taronja, parpelleig lent
//
typedef enum {
  ESTAT_INIT,
  ESTAT_WIFI_CONNECTANT,
  ESTAT_WIFI_OK_MQTT_CONN,
  ESTAT_TOT_OK,
  ESTAT_WIFI_OK_MQTT_PERDUT
} EstatSistema;

// ---- Variables globals ----
SemaphoreHandle_t dataMutex    = NULL;
EstatSistema      estatActual  = ESTAT_INIT;
bool              mqttRxFlash  = false;
bool              stateRestored = false;

WiFiMulti                   wifiMulti;
Adafruit_NeoPixel           pixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
Mycila::MQTT                mqtt;
temperature_sensor_handle_t hTempSensor = NULL;

OneWire           oneWire(PIN_DS18B20);
DallasTemperature ds18b20(&oneWire);

// LED state
const int LEDS[4]     = {PIN_LED1, PIN_LED2, PIN_LED3, PIN_LED4};
bool      ledState[4] = {false, false, false, false};

// Actuadors state
bool triacState  = false;
bool releState   = false;

// Buzzer state (momentani, s'apaga automàticament)
uint32_t buzzerEndTime = 0;

// Polsadors state
const int POLSADORS[4]     = {PIN_POLSADOR1, PIN_POLSADOR2, PIN_POLSADOR3, PIN_POLSADOR4};
bool      polsadorAnt[4]   = {HIGH, HIGH, HIGH, HIGH};
uint32_t  polsadorTemps[4] = {0, 0, 0, 0};


// ---- Helper: timestamp NTP ----
String getTimestamp() {
  struct tm ti;
  if (!getLocalTime(&ti, 100)) return "--:--:--";
  char buf[25];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ti);
  return String(buf);
}

// ---- Helper: publica estat retained LEDs + actuadors ----
void publishRetainedState() {
  char json[80];
  snprintf(json, sizeof(json), "{\"leds\":[%d,%d,%d,%d],\"triac\":%d,\"rele\":%d}",
    ledState[0], ledState[1], ledState[2], ledState[3],
    triacState ? 1 : 0,
    releState ? 1 : 0);
  mqtt.publish(TOPIC_LEDS, json, true);
}


// ============================================================
//  TASCA: Neopixel
//  Visualitza l'estat del sistema amb colors i parpelleigs.
//  Flash lila (100 ms) no bloquejant: usa millis() en lloc
//  de vTaskDelay, igual que el timer del complex per al buzzer.
//  Prioritat 3 · Core 1 · Stack 2048
// ============================================================
void neopixelTask(void* param) {
  pixel.begin();
  pixel.setBrightness(128);  // 50 % de brillantor màxima
  pixel.clear();
  pixel.show();

  bool     toggle     = false;
  uint32_t flashUntil = 0;

  while (true) {
    // Lectura atòmica de l'estat protegit
    EstatSistema estat;
    bool flash = false;

    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(20))) {
      estat = estatActual;
      if (mqttRxFlash) {
        flash       = true;
        mqttRxFlash = false;
      }
      xSemaphoreGive(dataMutex);
    } else {
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }

    if (flash) flashUntil = millis() + 100;

    if (millis() < flashUntil) {
      pixel.setPixelColor(0, pixel.Color(255, 0, 255));
      pixel.show();
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }

    switch (estat) {

      case ESTAT_INIT:
        pixel.setPixelColor(0, pixel.Color(255, 255, 255));
        pixel.show();
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

      case ESTAT_WIFI_CONNECTANT:
        toggle = !toggle;
        pixel.setPixelColor(0, toggle ? pixel.Color(0, 0, 255) : pixel.Color(0, 0, 0));
        pixel.show();
        vTaskDelay(pdMS_TO_TICKS(500));
        break;

      case ESTAT_WIFI_OK_MQTT_CONN:
        toggle = !toggle;
        pixel.setPixelColor(0, toggle ? pixel.Color(255, 200, 0) : pixel.Color(0, 0, 0));
        pixel.show();
        vTaskDelay(pdMS_TO_TICKS(125));
        break;

      case ESTAT_TOT_OK:
        pixel.setPixelColor(0, pixel.Color(0, 255, 0));
        pixel.show();
        vTaskDelay(pdMS_TO_TICKS(100));
        pixel.clear();
        pixel.show();
        vTaskDelay(pdMS_TO_TICKS(900));
        break;

      case ESTAT_WIFI_OK_MQTT_PERDUT:
        toggle = !toggle;
        pixel.setPixelColor(0, toggle ? pixel.Color(255, 80, 0) : pixel.Color(0, 0, 0));
        pixel.show();
        vTaskDelay(pdMS_TO_TICKS(500));
        break;
    }
  }
}


// ============================================================
//  TASCA: Polsadors
//  Llegeix els 4 polsadors (GPIO 39-42) amb pull-up intern i
//  debounce per software. Publica events PREMUT i ALLIBERAT.
//  Prioritat 3 · Core 1 · Stack 2048
// ============================================================
void polsadorsTask(void* param) {
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));

    for (int i = 0; i < 4; i++) {
      bool lectura = digitalRead(POLSADORS[i]);

      if (lectura != polsadorAnt[i] && !polsadorTemps[i]) {
        polsadorTemps[i] = millis();
      }

      if (polsadorTemps[i] && (millis() - polsadorTemps[i]) >= DEBOUNCE_MS) {
        bool estable = digitalRead(POLSADORS[i]);
        if (estable != polsadorAnt[i]) {
          polsadorAnt[i] = estable;
          if (estable == LOW) {
            Serial.printf("[POLS] Polsador %d PREMUT\n", i + 1);
            if (mqtt.isConnected()) {
              char json[48];
              snprintf(json, sizeof(json), "{\"boto\":%d,\"event\":\"PREMUT\"}", i + 1);
              mqtt.publish(TOPIC_POLSADORS, json);
            }
          } else {
            Serial.printf("[POLS] Polsador %d ALLIBERAT\n", i + 1);
            if (mqtt.isConnected()) {
              char json[48];
              snprintf(json, sizeof(json), "{\"boto\":%d,\"event\":\"ALLIBERAT\"}", i + 1);
              mqtt.publish(TOPIC_POLSADORS, json);
            }
          }
        }
        polsadorTemps[i] = 0;
      }
    }
  }
}


// ============================================================
//  TASCA: Temperatura interna
//  Llegeix el sensor tèrmic intern de l'ESP32-S3 i publica
//  el valor cada INTERVAL_TEMP ms.
//  Prioritat 4 · Core 1 · Stack 4096
// ============================================================
void tempInternaTask(void* param) {
  temperature_sensor_config_t cfg = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 50);
  temperature_sensor_install(&cfg, &hTempSensor);
  temperature_sensor_enable(hTempSensor);

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(INTERVAL_TEMP));

    float temp = 0.0f;
    temperature_sensor_get_celsius(hTempSensor, &temp);
    Serial.printf("[TEMP INT] %.2f °C\n", temp);

    if (!mqtt.isConnected()) continue;

    char payload[10];
    snprintf(payload, sizeof(payload), "%.2f", temp);
    mqtt.publish(TOPIC_TEMP_INT, payload);
  }
}


// ============================================================
//  TASCA: Temperatura DS18B20
//  Llegeix el sensor extern DS18B20 a GPIO16 i publica
//  el valor cada INTERVAL_TEMP ms.
//  Prioritat 4 · Core 1 · Stack 4096
// ============================================================
void tempDS18B20Task(void* param) {
  ds18b20.begin();
  ds18b20.setResolution(DS18B20_RESOLUTION);
  ds18b20.setWaitForConversion(false);

  uint32_t convTime = ds18b20.millisToWaitForConversion();

  ds18b20.requestTemperatures();
  vTaskDelay(pdMS_TO_TICKS(convTime));

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(INTERVAL_TEMP));

    ds18b20.requestTemperatures();
    vTaskDelay(pdMS_TO_TICKS(convTime));

    float temp = ds18b20.getTempCByIndex(0);

    if (temp == DEVICE_DISCONNECTED_C) {
      Serial.println("[TEMP DS] Sensor no detectat");
      continue;
    }

    Serial.printf("[TEMP DS] %.2f °C\n", temp);

    if (!mqtt.isConnected()) continue;

    char payload[10];
    snprintf(payload, sizeof(payload), "%.2f", temp);
    mqtt.publish(TOPIC_TEMP_DS, payload);
  }
}


// ============================================================
//  TASCA PRINCIPAL: WiFi + MQTT + publicació d'estat
//  Prioritat 5 · Core 1 · Stack 8192
// ============================================================
void mainTask(void* param) {

  // --- WiFi ---

  wifiMulti.addAP(WIFI_SSID_1, WIFI_PASS_1);
  if (strlen(WIFI_SSID_2) > 0) wifiMulti.addAP(WIFI_SSID_2, WIFI_PASS_2);
  if (strlen(WIFI_SSID_3) > 0) wifiMulti.addAP(WIFI_SSID_3, WIFI_PASS_3);

  if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
    estatActual = ESTAT_WIFI_CONNECTANT;
    xSemaphoreGive(dataMutex);
  }
  Serial.println("[WiFi] Connectant...");

  while (WiFi.status() != WL_CONNECTED) {
    uint32_t t0 = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - t0) < 10000) {
      wifiMulti.run();
      vTaskDelay(pdMS_TO_TICKS(500));
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[WiFi] Cap xarxa disponible, reintentant en 10 s...");
      vTaskDelay(pdMS_TO_TICKS(10000));
    }
  }

  Serial.printf("[WiFi] Connectat a '%s'  IP: %s\n",
    WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());

  // --- NTP ---
  configTime(GMT_OFFSET_SEC, DST_OFFSET_SEC, NTP_SERVER);
  Serial.println("[NTP] Sincronitzant hora...");
  {
    struct tm ti;
    if (getLocalTime(&ti, 5000))
      Serial.printf("[NTP] Hora: %s\n", getTimestamp().c_str());
    else
      Serial.println("[NTP] Sincronització fallida (continuem sense hora)");
  }

  // --- MQTT ---

  mqtt.onConnect([]() {
    Serial.println("[MQTT] Connectat al broker");
    mqtt.publish(TOPIC_ONLINE, "1", true);
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50))) {
      estatActual   = ESTAT_TOT_OK;
      stateRestored = false;
      xSemaphoreGive(dataMutex);
    }

    // Subscriure's al tòpic de comandaments
    mqtt.subscribe(TOPIC_ORDRE, [](const std::string& topic,
                                   const std::string_view& payload) {
      String c = String(payload.data(), payload.size());
      c.trim();
      Serial.printf("[MQTT RX] %s → %s\n", topic.c_str(), c.c_str());

      // ---- Comandes LEDs ----
      if (c.startsWith("led")) {
        int idx = c.substring(3, 4).toInt() - 1;
        if (idx >= 0 && idx < 4) {
          bool newVal;

          if (!xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50))) return;
          if      (c.endsWith("ON"))  ledState[idx] = true;
          else if (c.endsWith("OFF")) ledState[idx] = false;
          else if (c.endsWith("TOG")) ledState[idx] = !ledState[idx];
          else { xSemaphoreGive(dataMutex); return; }
          newVal = ledState[idx];
          mqttRxFlash = true;
          xSemaphoreGive(dataMutex);

          digitalWrite(LEDS[idx], newVal);
          publishRetainedState();
          Serial.printf("[LEDS] %s → led%d=%d\n", c.c_str(), idx + 1, newVal);
        }
        return;
      }

      // ---- Comandes actuadors ----
      if (c.startsWith("triac ")) {
        bool newVal = false;
        if      (c.endsWith("ON"))  newVal = true;
        else if (c.endsWith("OFF")) newVal = false;
        else if (c.endsWith("TOG")) { newVal = !triacState; }
        else return;

        triacState = newVal;
        digitalWrite(PIN_TRIAC, triacState);
        if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50))) {
          mqttRxFlash = true;
          xSemaphoreGive(dataMutex);
        }
        publishRetainedState();
        Serial.printf("[TRIAC] %s\n", c.c_str());
        return;
      }

      if (c.startsWith("rele ")) {
        bool newVal = false;
        if      (c.endsWith("ON"))  newVal = true;
        else if (c.endsWith("OFF")) newVal = false;
        else if (c.endsWith("TOG")) { newVal = !releState; }
        else return;

        releState = newVal;
        digitalWrite(PIN_RELE, releState);
        if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50))) {
          mqttRxFlash = true;
          xSemaphoreGive(dataMutex);
        }
        publishRetainedState();
        Serial.printf("[RELE] %s\n", c.c_str());
        return;
      }

      if (c.startsWith("buzzer ")) {
        int s1 = c.indexOf(' ', 7);
        int s2 = c.indexOf(' ', s1 + 1);
        if (s1 > 0 && s2 > 0) {
          uint32_t freq = c.substring(s1 + 1, s2).toInt();
          uint32_t dur  = c.substring(s2 + 1).toInt();
          if (dur > 40) dur = 40;

          ledcAttach(PIN_BUZZER, freq, 8);
          if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50))) {
            buzzerEndTime = millis() + dur;
            mqttRxFlash   = true;
            xSemaphoreGive(dataMutex);
          }
          Serial.printf("[BUZZER] %u Hz, %u ms\n", freq, dur);
        }
        return;
      }
    });

    // Subscriure's a l'estat retained per restaurar-lo en reconnectar
    mqtt.subscribe(TOPIC_LEDS, [](const std::string& topic,
                                  const std::string_view& payload) {
      if (stateRestored) return;

      StaticJsonDocument<96> doc;
      if (deserializeJson(doc, payload.data(), payload.size()) != DeserializationError::Ok) {
        Serial.println("[RETAINED] Error parsejant JSON");
        return;
      }
      JsonArray arr = doc["leds"].as<JsonArray>();

      bool snapLEDs[4] = {};
      if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50))) {
        for (int i = 0; i < 4 && i < (int)arr.size(); i++) {
          ledState[i] = arr[i].as<bool>();
          snapLEDs[i] = ledState[i];
        }
        // Restaurar triac i rele del retained si existeixen
        triacState = doc["triac"] | false;
        releState  = doc["rele"]  | false;
        stateRestored = true;
        xSemaphoreGive(dataMutex);
      }
      for (int i = 0; i < 4; i++) digitalWrite(LEDS[i], snapLEDs[i]);
      digitalWrite(PIN_TRIAC, triacState);
      digitalWrite(PIN_RELE,  releState);
      Serial.printf("[RETAINED] Estat restaurat: leds=[%d,%d,%d,%d] triac=%d rele=%d\n",
        snapLEDs[0], snapLEDs[1], snapLEDs[2], snapLEDs[3],
        triacState, releState);
    });
  });

  // Client ID únic basat en la MAC completa
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  String clientId = String(MQTT_CLIENT_ID) + "-" + mac;

  Mycila::MQTT::Config mqttCfg;
  mqttCfg.server      = MQTT_BROKER;
  mqttCfg.port        = MQTT_PORT;
  mqttCfg.clientId    = clientId.c_str();
  mqttCfg.willTopic = TOPIC_ONLINE;
  mqtt.begin(mqttCfg);

  if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50))) {
    estatActual = ESTAT_WIFI_OK_MQTT_CONN;
    xSemaphoreGive(dataMutex);
  }
  Serial.println("[MQTT] Connectant al broker...");

  // --- Bucle principal ---
  uint32_t tEstat = 0;

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(100));

    // Control fi del buzzer (apagar quan expira la durada)
    if (buzzerEndTime > 0 && millis() >= buzzerEndTime) {
      if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10))) {
        if (buzzerEndTime > 0 && millis() >= buzzerEndTime) {
          ledcDetach(PIN_BUZZER);
          buzzerEndTime = 0;
        }
        xSemaphoreGive(dataMutex);
      }
    }

    // Reconectar WiFi si s'ha perdut
    if (WiFi.status() != WL_CONNECTED) {
      if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10))) {
        if (estatActual != ESTAT_WIFI_CONNECTANT) {
          estatActual = ESTAT_WIFI_CONNECTANT;
          Serial.println("[WiFi] Connexió perduda, reconnectant...");
        }
        xSemaphoreGive(dataMutex);
      }
      wifiMulti.run();
      continue;
    }

    // WiFi recuperat
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10))) {
      if (estatActual == ESTAT_WIFI_CONNECTANT) {
        estatActual = ESTAT_WIFI_OK_MQTT_CONN;
        Serial.printf("[WiFi] Reconnectat a '%s'\n", WiFi.SSID().c_str());
      }
      xSemaphoreGive(dataMutex);
    }

    // Detectar pèrdua de MQTT
    if (!mqtt.isConnected()) {
      if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10))) {
        if (estatActual == ESTAT_TOT_OK) {
          estatActual = ESTAT_WIFI_OK_MQTT_PERDUT;
          Serial.println("[MQTT] Connexió perduda (reconnexió automàtica en curs)");
        }
        xSemaphoreGive(dataMutex);
      }
    }

    // Publicar l'estat del dispositiu cada INTERVAL_ESTAT ms
    if (mqtt.isConnected() && (millis() - tEstat) >= INTERVAL_ESTAT) {
      tEstat = millis();

      int ls[4] = {};
      bool t, r;
      if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10))) {
        for (int i = 0; i < 4; i++) ls[i] = ledState[i];
        t = triacState;
        r = releState;
        xSemaphoreGive(dataMutex);
      }

      int ps[4];
      for (int i = 0; i < 4; i++) ps[i] = digitalRead(POLSADORS[i]) ? 0 : 1;

      char json[200];
      snprintf(json, sizeof(json),
        "{\"time\":\"%s\",\"ip\":\"%s\",\"rssi\":%d,\"uptime\":%lu,"
        "\"leds\":[%d,%d,%d,%d],\"triac\":%d,\"rele\":%d,\"buzzer\":0,"
        "\"polsadors\":[%d,%d,%d,%d]}",
        getTimestamp().c_str(),
        WiFi.localIP().toString().c_str(),
        WiFi.RSSI(),
        millis() / 1000UL,
        ls[0], ls[1], ls[2], ls[3],
        t ? 1 : 0,
        r ? 1 : 0,
        ps[0], ps[1], ps[2], ps[3]
      );
      mqtt.publish(TOPIC_ESTAT, json);
      Serial.printf("[MQTT TX] %s\n", json);
    }
  }
}


// ============================================================
//  setup() i loop()
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== ESP32-S3 Template v7 ===");

  // Mutex
  dataMutex = xSemaphoreCreateMutex();

  // Configurar LEDs
  for (int i = 0; i < 4; i++) {
    pinMode(LEDS[i], OUTPUT);
    digitalWrite(LEDS[i], LOW);
  }

  // Configurar actuadors
  pinMode(PIN_TRIAC, OUTPUT);
  digitalWrite(PIN_TRIAC, LOW);
  pinMode(PIN_RELE, OUTPUT);
  digitalWrite(PIN_RELE, LOW);
  // Buzzer sense pinMode (ledcAttach ho fa automàticament)

  // Configurar polsadors (pull-up intern)
  for (int i = 0; i < 4; i++) {
    pinMode(POLSADORS[i], INPUT_PULLUP);
  }

  // Crear les cinc tasques FreeRTOS
  //                           funció               nom      stack   param  prio  handle  core
  xTaskCreatePinnedToCore(neopixelTask,           "neo",   3072, NULL,  3, NULL, 1);
  xTaskCreatePinnedToCore(polsadorsTask,          "pols",  2048, NULL,  3, NULL, 1);
  xTaskCreatePinnedToCore(tempInternaTask,        "temp",  4096, NULL,  4, NULL, 1);
  xTaskCreatePinnedToCore(tempDS18B20Task,        "ds18",  4096, NULL,  4, NULL, 1);
  xTaskCreatePinnedToCore(mainTask,               "main",  8192, NULL,  5, NULL, 1);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}

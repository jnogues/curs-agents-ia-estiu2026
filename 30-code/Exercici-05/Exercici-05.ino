/*
 * esp32s3_template.ino
 *
 * v5 — Template ESP32-S3 per a Arduino IDE
 * Funcionalitats: WiFi multi-xarxa · MQTT · Neopixel · Temperatura interna + DS18B20 · NTP
 * Arquitectura:   4 tasques FreeRTOS (loop() queda buit)
 *
 * Llibreries necessàries (Arduino Library Manager):
 *   - "MycilaMQTT"          per mathieucarbou
 *   - "Adafruit NeoPixel"   per Adafruit
 *   - "OneWire"             per Jim Studt
 *   - "DallasTemperature"   per Miles Burton
 *   - "ArduinoJson"         per Benoit Blanchon      ← NOU (v5)
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

// Hardware
#define NEOPIXEL_PIN    48   // GPIO del LED WS2812B integrat
#define NEOPIXEL_COUNT  1
#define PIN_DS18B20     16
// LEDs externs (actius per HIGH). Cada LED es controla per MQTT:
//   led1 (GPIO7)   led2 (GPIO15)   led3 (GPIO47)   led4 (GPIO21)
// Comandes: "led1 ON", "led1 OFF", "led1 TOG" (idem led2..4)
#define PIN_LED1         7
#define PIN_LED2        15
#define PIN_LED3        47
#define PIN_LED4        21

// Precisió del DS18B20 (9..12 bits). 12 bits = 0.0625 °C, 750 ms de conversió.
#define DS18B20_RESOLUTION  12

// Intervals de publicació MQTT (mil·lisegons)
#define INTERVAL_ESTAT  5000
#define INTERVAL_TEMP   10000

// NTP
#define NTP_SERVER      "pool.ntp.org"
#define GMT_OFFSET_SEC  3600
#define DST_OFFSET_SEC  3600

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
// Llegir estat retained dels LEDs:
//   mosquitto_sub -h 46.224.116.35 -t "/esp32s3/alumneXX/stateLEDS" -C 1
//
// Tòpics de telemetria (només lectura):
//   /esp32s3/alumneXX/estat          → cada 5s  {"time":...,"ip":...,"rssi":...,"uptime":...,"leds":[...]}
//   /esp32s3/alumneXX/temperaturaINT → cada 10s
//   /esp32s3/alumneXX/temperaturaDS  → cada 10s
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
// estatActual i mqttRxFlash es llegeixen/escriuen des de múltiples tasques
// i des dels callbacks MQTT (fil intern de MycilaMQTT). Protegides per dataMutex.
SemaphoreHandle_t dataMutex    = NULL;
EstatSistema      estatActual  = ESTAT_INIT;
bool              mqttRxFlash  = false;
bool              stateRestored = false;  // true després de restaurar l'estat retained dels LEDs

WiFiMulti                   wifiMulti;
Adafruit_NeoPixel           pixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
Mycila::MQTT                mqtt;
temperature_sensor_handle_t hTempSensor = NULL;

OneWire           oneWire(PIN_DS18B20);
DallasTemperature ds18b20(&oneWire);

// ledState[] es modifica per MQTT i es publica a stateLEDS (retained)
const int LEDS[4]     = {PIN_LED1, PIN_LED2, PIN_LED3, PIN_LED4};
bool      ledState[4] = {false, false, false, false};


// ---- Helper: timestamp NTP ----
String getTimestamp() {
  struct tm ti;
  if (!getLocalTime(&ti, 100)) return "--:--:--";
  char buf[25];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ti);
  return String(buf);
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

    // Arrancar el flash si cal
    if (flash) flashUntil = millis() + 100;

    // Flash lila prioritari: no bloqueja la tasca, la state machine
    // reprèn immediatament quan expira el temps
    if (millis() < flashUntil) {
      pixel.setPixelColor(0, pixel.Color(255, 0, 255));
      pixel.show();
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }

    switch (estat) {

      case ESTAT_INIT:
        pixel.setPixelColor(0, pixel.Color(255, 255, 255));  // blanc fix
        pixel.show();
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

      case ESTAT_WIFI_CONNECTANT:
        toggle = !toggle;
        pixel.setPixelColor(0, toggle ? pixel.Color(0, 0, 255) : pixel.Color(0, 0, 0));
        pixel.show();
        vTaskDelay(pdMS_TO_TICKS(500));          // 1 Hz
        break;

      case ESTAT_WIFI_OK_MQTT_CONN:
        toggle = !toggle;
        pixel.setPixelColor(0, toggle ? pixel.Color(255, 200, 0) : pixel.Color(0, 0, 0));
        pixel.show();
        vTaskDelay(pdMS_TO_TICKS(125));          // 4 Hz
        break;

      case ESTAT_TOT_OK:
        // Batec: encès breu (100 ms), apagat llarg (900 ms) → 1 pols per segon
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
        vTaskDelay(pdMS_TO_TICKS(500));          // 1 Hz
        break;
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

  // Reintenta indefinidament fins connectar (cada 10 s si no hi ha xarxa)
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

  // Callback: s'executa cada vegada que es (re)connecta al broker
  mqtt.onConnect([]() {
    Serial.println("[MQTT] Connectat al broker");
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50))) {
      estatActual   = ESTAT_TOT_OK;
      stateRestored = false;  // permet restaurar l'estat retained en cada (re)connexió
      xSemaphoreGive(dataMutex);
    }

    // Subscriure's al tòpic de comandaments
    mqtt.subscribe(TOPIC_ORDRE, [](const std::string& topic,
                                   const std::string_view& payload) {
      String c = String(payload.data(), payload.size());
      c.trim();
      Serial.printf("[MQTT RX] %s → %s\n", topic.c_str(), c.c_str());

      // Comandes LEDs: "led1 ON" | "led1 OFF" | "led1 TOG" (led2..4 igual)
      if (c.startsWith("led")) {
        int idx = c.substring(3, 4).toInt() - 1;
        if (idx >= 0 && idx < 4) {
          char json[40];
          bool newVal;

          // Escriptura de ledState i lectura consistent per al JSON dins del mateix mutex
          if (!xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50))) return;
          if      (c.endsWith("ON"))  ledState[idx] = true;
          else if (c.endsWith("OFF")) ledState[idx] = false;
          else if (c.endsWith("TOG")) ledState[idx] = !ledState[idx];
          else { xSemaphoreGive(dataMutex); return; }
          newVal = ledState[idx];
          snprintf(json, sizeof(json), "{\"leds\":[%d,%d,%d,%d]}",
            ledState[0], ledState[1], ledState[2], ledState[3]);
          mqttRxFlash = true;
          xSemaphoreGive(dataMutex);

          digitalWrite(LEDS[idx], newVal);
          mqtt.publish(TOPIC_LEDS, json, true);
          Serial.printf("[LEDS] %s → %s\n", c.c_str(), json);
        }
      }
    });

    // Subscriure's a l'estat retained dels LEDs per restaurar-lo en reconnectar.
    // stateRestored evita que els missatges posteriors (dels nostres propis canvis)
    // tornin a executar la restauració innecessàriament.
    mqtt.subscribe(TOPIC_LEDS, [](const std::string& topic,
                                  const std::string_view& payload) {
      if (stateRestored) return;

      StaticJsonDocument<64> doc;
      if (deserializeJson(doc, payload.data(), payload.size()) != DeserializationError::Ok) {
        Serial.println("[LEDS] Error parsejant JSON retained");
        return;
      }
      JsonArray arr = doc["leds"].as<JsonArray>();

      bool snap[4] = {};
      if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50))) {
        for (int i = 0; i < 4 && i < (int)arr.size(); i++) {
          ledState[i] = arr[i].as<bool>();
          snap[i]     = ledState[i];
        }
        stateRestored = true;
        xSemaphoreGive(dataMutex);
      }
      for (int i = 0; i < 4; i++) digitalWrite(LEDS[i], snap[i]);
      Serial.println("[LEDS] Estat restaurat des del broker");
    });
  });

  // Client ID únic basat en la MAC completa (garanteix unicitat sense configuració manual)
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  String clientId = String(MQTT_CLIENT_ID) + "-" + mac;

  Mycila::MQTT::Config mqttCfg;
  mqttCfg.server   = MQTT_BROKER;
  mqttCfg.port     = MQTT_PORT;
  mqttCfg.clientId = clientId.c_str();
  mqtt.begin(mqttCfg);  // la connexió i reconexió automàtica les gestiona la llibreria

  if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50))) {
    estatActual = ESTAT_WIFI_OK_MQTT_CONN;
    xSemaphoreGive(dataMutex);
  }
  Serial.println("[MQTT] Connectant al broker...");

  // --- Bucle principal ---
  uint32_t tEstat = 0;

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(100));

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

    // WiFi recuperat — actualitzar estat si venia de reconnexió
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
      if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10))) {
        for (int i = 0; i < 4; i++) ls[i] = ledState[i];
        xSemaphoreGive(dataMutex);
      }

      char json[160];
      snprintf(json, sizeof(json),
        "{\"time\":\"%s\",\"ip\":\"%s\",\"rssi\":%d,\"uptime\":%lu,\"leds\":[%d,%d,%d,%d]}",
        getTimestamp().c_str(),
        WiFi.localIP().toString().c_str(),
        WiFi.RSSI(),
        millis() / 1000UL,
        ls[0], ls[1], ls[2], ls[3]
      );
      mqtt.publish(TOPIC_ESTAT, json);
      Serial.printf("[MQTT TX] %s\n", json);
    }
  }
}


// ============================================================
//  setup() i loop() — no cal modificar res aquí sota
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== ESP32-S3 Template v5 ===");

  // Mutex creat ABANS de les tasques (igual que el complex crea els mutex
  // abans de crear els sprites i les tasques)
  dataMutex = xSemaphoreCreateMutex();

  // Configurar LEDs
  for (int i = 0; i < 4; i++) {
    pinMode(LEDS[i], OUTPUT);
    digitalWrite(LEDS[i], LOW);
  }

  // Crear les quatre tasques FreeRTOS
  //                           funció            nom      stack   param  prio  handle  core
  xTaskCreatePinnedToCore(neopixelTask,      "neo",   3072, NULL,  3, NULL, 1);
  xTaskCreatePinnedToCore(tempInternaTask,   "temp",  4096, NULL,  4, NULL, 1);
  xTaskCreatePinnedToCore(tempDS18B20Task,   "ds18",  4096, NULL,  4, NULL, 1);
  xTaskCreatePinnedToCore(mainTask,          "main",  8192, NULL,  5, NULL, 1);
}

void loop() {
  // Tot el treball és a les tasques FreeRTOS.
  // Suspenem loop() per no consumir CPU.
  vTaskDelay(portMAX_DELAY);
}

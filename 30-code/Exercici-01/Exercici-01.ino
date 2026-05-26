/*
 * esp32s3_template.ino
 *
 * Template ESP32-S3 per a Arduino IDE
 * Funcionalitats: WiFi multi-xarxa · MQTT · Neopixel · Temperatura interna
 * Arquitectura:   3 tasques FreeRTOS (loop() queda buit)
 *
 * Llibreries necessàries (Arduino Library Manager):
 *   - "MycilaMQTT"       per mathieucarbou
 *   - "Adafruit NeoPixel" per Adafruit
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

// Hardware
#define NEOPIXEL_PIN    48   // GPIO del LED WS2812B integrat
#define NEOPIXEL_COUNT  1

// Intervals de publicació MQTT (mil·lisegons)
#define INTERVAL_ESTAT  5000
#define INTERVAL_TEMP   10000

// ============================================================

#include <WiFi.h>
#include <WiFiMulti.h>
#include <Preferences.h>
#include <Adafruit_NeoPixel.h>
#include <MycilaMQTT.h>
#include "driver/temperature_sensor.h"

// ---- Codificació d'estats del sistema ----
//
//  ESTAT_INIT                → LED blanc fix
//  ESTAT_WIFI_CONNECTANT     → LED blau, parpelleig lent  (1 Hz)
//  ESTAT_WIFI_OK_MQTT_CONN   → LED groc, parpelleig ràpid (4 Hz)
//  ESTAT_TOT_OK              → LED verd, batec lent (1 Hz)
//  ESTAT_WIFI_FALLAT         → (reservat, no s'usa actualment)
//  ESTAT_WIFI_OK_MQTT_PERDUT → LED taronja, parpelleig lent
//
typedef enum {
  ESTAT_INIT,
  ESTAT_WIFI_CONNECTANT,
  ESTAT_WIFI_OK_MQTT_CONN,
  ESTAT_TOT_OK,
  ESTAT_WIFI_FALLAT,
  ESTAT_WIFI_OK_MQTT_PERDUT
} EstatSistema;

// ---- Variables globals ----
volatile EstatSistema estatActual = ESTAT_INIT;

WiFiMulti                   wifiMulti;
Preferences                 prefs;
Adafruit_NeoPixel           pixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
Mycila::MQTT                mqtt;
temperature_sensor_handle_t hTempSensor = NULL;


// ============================================================
//  TASCA: Neopixel
//  Visualitza l'estat del sistema amb colors i parpelleigs.
//  Prioritat 3 · Core 1 · Stack 2048
// ============================================================
void neopixelTask(void* param) {
  pixel.begin();
  pixel.setBrightness(128);  // 50 % de brillantor màxima
  pixel.clear();
  pixel.show();

  bool toggle = false;

  while (true) {
    switch (estatActual) {

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

      case ESTAT_WIFI_FALLAT:
        toggle = !toggle;
        pixel.setPixelColor(0, toggle ? pixel.Color(255, 0, 0) : pixel.Color(0, 0, 0));
        pixel.show();
        vTaskDelay(pdMS_TO_TICKS(125));          // 4 Hz
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
  // Inicialitzar el sensor de temperatura intern
  temperature_sensor_config_t cfg = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 50);
  temperature_sensor_install(&cfg, &hTempSensor);
  temperature_sensor_enable(hTempSensor);

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(INTERVAL_TEMP));

    if (!mqtt.isConnected()) continue;  // no publiquem si no hi ha connexió

    float temp = 0.0f;
    temperature_sensor_get_celsius(hTempSensor, &temp);

    char payload[10];
    snprintf(payload, sizeof(payload), "%.2f", temp);
    mqtt.publish(TOPIC_TEMP_INT, payload);

    Serial.printf("[TEMP INT] %.2f °C\n", temp);
  }
}


// ============================================================
//  TASCA PRINCIPAL: WiFi + MQTT + publicació d'estat
//  Prioritat 5 · Core 1 · Stack 8192
// ============================================================
void mainTask(void* param) {

  // --- WiFi ---

  // Recuperar la darrera xarxa que va funcionar (guardada a la memòria NVS)
  prefs.begin("wifi", true);
  String ultimaXarxa = prefs.getString("ultima", "");
  prefs.end();

  if (ultimaXarxa.length() > 0)
    Serial.printf("[WiFi] Última xarxa connectada: %s\n", ultimaXarxa.c_str());

  // Afegir les xarxes disponibles
  wifiMulti.addAP(WIFI_SSID_1, WIFI_PASS_1);
  if (strlen(WIFI_SSID_2) > 0) wifiMulti.addAP(WIFI_SSID_2, WIFI_PASS_2);
  if (strlen(WIFI_SSID_3) > 0) wifiMulti.addAP(WIFI_SSID_3, WIFI_PASS_3);

  estatActual = ESTAT_WIFI_CONNECTANT;
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

  // Guardar la xarxa actual per al proper arrencament
  prefs.begin("wifi", false);
  prefs.putString("ultima", WiFi.SSID());
  prefs.end();

  // --- MQTT ---

  // Callback: s'executa cada vegada que es (re)connecta al broker
  mqtt.onConnect([]() {
    Serial.println("[MQTT] Connectat al broker");
    estatActual = ESTAT_TOT_OK;

    // Subscriure's al tòpic d'ordres
    mqtt.subscribe(TOPIC_ORDRE, [](const std::string& topic,
                                   const std::string_view& payload) {
      Serial.printf("[MQTT RX] %s → %.*s\n",
        topic.c_str(), (int)payload.size(), payload.data());

      // TODO: implementa aquí la lògica de recepció d'ordres
      // Exemple: if (payload == "LED_ON") { ... }
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

  estatActual = ESTAT_WIFI_OK_MQTT_CONN;
  Serial.println("[MQTT] Connectant al broker...");

  // --- Bucle principal ---
  uint32_t tEstat = 0;

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(100));

    // Reconectar WiFi si s'ha perdut
    if (WiFi.status() != WL_CONNECTED) {
      if (estatActual != ESTAT_WIFI_CONNECTANT) {
        estatActual = ESTAT_WIFI_CONNECTANT;
        Serial.println("[WiFi] Connexió perduda, reconnectant...");
      }
      wifiMulti.run();
      continue;
    }
    // WiFi recuperat — actualitzar estat si venia de reconnexió
    if (estatActual == ESTAT_WIFI_CONNECTANT) {
      estatActual = ESTAT_WIFI_OK_MQTT_CONN;
      Serial.printf("[WiFi] Reconnectat a '%s'\n", WiFi.SSID().c_str());
    }

    // Detectar pèrdua de MQTT
    if (!mqtt.isConnected() && estatActual == ESTAT_TOT_OK) {
      estatActual = ESTAT_WIFI_OK_MQTT_PERDUT;
      Serial.println("[MQTT] Connexió perduda (reconnexió automàtica en curs)");
    }

    // Publicar l'estat del dispositiu cada INTERVAL_ESTAT ms
    if (mqtt.isConnected() && (millis() - tEstat) >= INTERVAL_ESTAT) {
      tEstat = millis();

      char json[128];
      snprintf(json, sizeof(json),
        "{\"ip\":\"%s\",\"rssi\":%d,\"uptime\":%lu}",
        WiFi.localIP().toString().c_str(),
        WiFi.RSSI(),
        millis() / 1000UL
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
  Serial.println("\n=== ESP32-S3 Template ===");

  // Crear les tres tasques FreeRTOS
  //                           funció          nom      stack   param  prio  handle  core
  xTaskCreatePinnedToCore(neopixelTask,    "neo",   2048, NULL,  3, NULL, 1);
  xTaskCreatePinnedToCore(tempInternaTask, "temp",  4096, NULL,  4, NULL, 1);
  xTaskCreatePinnedToCore(mainTask,        "main",  8192, NULL,  5, NULL, 1);
}

void loop() {
  // Tot el treball és a les tasques FreeRTOS.
  // Suspenem loop() per no consumir CPU.
  vTaskDelay(portMAX_DELAY);
}

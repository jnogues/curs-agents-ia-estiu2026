#pragma once

/*
  Protocol ESP-NOW ↔ MQTT
  ========================
  Format: JSON, màx ESPNOW_MAX_PAYLOAD bytes (limitació hardware ESP-NOW).

  Node ID: "{nom}_{3 últims bytes MAC}"  ex: "salon_AABBCC"
    - El nom és configurable per sketch (#define NODE_NAME).
    - Els 3 bytes de MAC garanteixen unicitat sense coordinació central.

  Nodes ESP8266 envien sempre en BROADCAST (FF:FF:FF:FF:FF:FF).
    Motiu: quan la gateway té WiFi actiu en mode WIFI_STA, el driver
    ESP32 no pot respondre ACKs unicast a dispositius externs a l'AP.
    La gateway filtra els missatges pel camp "id" del JSON.

  Fluxos:
    node → gateway : announce, sensor, event, state, ack
    gateway → node : command
*/

// ESP-NOW maximum payload size (hardware limit)
#define ESPNOW_MAX_PAYLOAD 250

// Message types (field "type")
#define MSG_ANNOUNCE  "announce"   // node → gateway: primer contacte, capacitats
#define MSG_SENSOR    "sensor"     // node → gateway: dades de sensors (periòdic)
#define MSG_EVENT     "event"      // node → gateway: esdeveniment puntual
#define MSG_STATE     "state"      // node → gateway: estat actual d'un actuador
#define MSG_COMMAND   "command"    // gateway → node: ordre
#define MSG_ACK       "ack"        // node → gateway: confirmació de comanda

// Node capabilities (field "caps" in announce)
#define CAP_SENSOR    "sensor"
#define CAP_COMMAND   "command"
#define CAP_EVENT     "event"

/*
  Exemples de missatges:

  announce:  { "id": "salon_AABBCC", "type": "announce", "caps": ["sensor","command"] }
  sensor:    { "id": "salon_AABBCC", "type": "sensor",   "data": {"temp": 22.5, "hum": 60} }
  event:     { "id": "porta_DDEEFF", "type": "event",    "data": {"trigger": "motion"} }
  command:   { "id": "llum1_112233", "type": "command",  "data": {"state": "on"} }
  ack:       { "id": "llum1_112233", "type": "ack",      "cmd": "command" }

  Topics MQTT:
    espnow/{id}/state  ← gateway publica sensor/event/state
    espnow/{id}/set    → gateway subscriu per enviar commands als nodes
*/

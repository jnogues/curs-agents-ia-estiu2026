/*
  basic_node.ino — Node mesh bàsic amb painlessMesh
  ==================================================
  El mateix sketch funciona a tots els nodes (ESP32 i ESP8266).
  Cada node s'anuncia a la xarxa, envia un missatge broadcast
  cada 5 segons i imprimeix els missatges que rep dels altres.

  Boards suportades:
    esp32:esp32:esp32s3    (ESP32-S3 DevKitC-1)
    esp8266:esp8266:nodemcuv2

  Compilar i flashejar:
    arduino-cli compile --fqbn esp32:esp32:esp32s3    basic_node/
    arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 basic_node/

    arduino-cli upload --fqbn esp32:esp32:esp32s3    --port /dev/ttyACM0 basic_node/
    arduino-cli upload --fqbn esp8266:esp8266:nodemcuv2 --port /dev/ttyUSB0 basic_node/
*/

#include <painlessMesh.h>

#define MESH_SSID   "mesh-lab"
#define MESH_PASS   "meshpass123"
#define MESH_PORT   5555

#define SEND_INTERVAL 5000  // ms entre missatges broadcast

Scheduler userScheduler;
painlessMesh mesh;

// --- Enviament periòdic ---
void sendMessage() {
    String msg = "Hola des de " + String(mesh.getNodeId());
    mesh.sendBroadcast(msg);
    Serial.printf("[SEND] %s\n", msg.c_str());
}

Task taskSend(SEND_INTERVAL, TASK_FOREVER, &sendMessage);

// --- Callbacks ---
void onReceive(uint32_t from, String &msg) {
    Serial.printf("[RECV] node %u → %s\n", from, msg.c_str());
}

void onNewConnection(uint32_t nodeId) {
    Serial.printf("[MESH] Node connectat:     %u  (xarxa: %u nodes)\n",
                  nodeId, mesh.getNodeList().size() + 1);
}

void onDroppedConnection(uint32_t nodeId) {
    Serial.printf("[MESH] Node desconnectat:  %u  (xarxa: %u nodes)\n",
                  nodeId, mesh.getNodeList().size() + 1);
}

void onChangedConnections() {
    Serial.printf("[MESH] Topologia actualitzada. Nodes: %u\n",
                  mesh.getNodeList().size() + 1);
}

// --- Setup / Loop ---
void setup() {
    Serial.begin(115200);

    mesh.setDebugMsgTypes(ERROR | STARTUP);
    mesh.init(MESH_SSID, MESH_PASS, &userScheduler, MESH_PORT);
    mesh.onReceive(&onReceive);
    mesh.onNewConnection(&onNewConnection);
    mesh.onDroppedConnection(&onDroppedConnection);
    mesh.onChangedConnections(&onChangedConnections);

    userScheduler.addTask(taskSend);
    taskSend.enable();

    Serial.printf("[INIT] Node ID: %u\n", mesh.getNodeId());
}

void loop() {
    mesh.update();
}

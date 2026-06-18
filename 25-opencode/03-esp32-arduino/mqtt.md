# MQTT (ESP32) — prompt per a OpenCode

Copia i enganxa això a OpenCode:

```
Genera un sketch d'Arduino per a ESP32 que:

1. Es connecti a WiFi
2. Es connecti a un broker MQTT (posa'l com a variable editble)
3. Es subscrigui al tòpic "esp32/led" i quan rebi "ON" encengui el GPIO2, i amb "OFF" l'apagui
4. Publiqui "hello" al tòpic "esp32/status" cada 10 segons

Utilitza la llibreria PubSubClient. Desa'l com a mqtt-test.ino, compila'l i puja'l.
Fes que el broker MQTT sigui configurable (IP i port).

FQBN: esp32:esp32:esp32-s3 o esp32:esp32:esp32
```

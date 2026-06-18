# MQTT (ESP8266) — prompt per a OpenCode

Copia i enganxa això a OpenCode:

```
Genera un sketch d'Arduino per a ESP8266 (NodeMCU) que:

1. Es connecti a WiFi
2. Es connecti a un broker MQTT (posa'l com a variable editable)
3. Es subscrigui al tòpic "esp8266/led" i quan rebi "ON" encengui el GPIO2 (LED integrat), i amb "OFF" l'apagui
4. Publiqui "hello" al tòpic "esp8266/status" cada 10 segons

Utilitza la llibreria PubSubClient. Desa'l com a mqtt-test.ino, compila'l i puja'l.
Fes que el broker MQTT sigui configurable (IP i port).

FQBN: esp8266:esp8266:nodemcuv2
```

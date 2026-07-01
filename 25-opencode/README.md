# 💻 Guia d'OpenCode

OpenCode és un agent de programació autònom, de codi obert i gratuït. T'ajuda a escriure, revisar i millorar codi directament des del terminal.

Perfecte per programar **ESP32** i **ESP8266** amb assistència d'IA!

---

## Com es treballa amb OpenCode

El flux de treball és sempre el mateix:

1. **Crea una carpeta** per al teu projecte (`mkdir Exercici1 && cd Exercici1`)
2. **Obre OpenCode** amb `opencode` (s'obre el TUI interactiu)
3. **Explica-li què vols** en català (ex: "Fes un blink per ESP32 al GPIO2")
4. **Surt amb Ctrl+C** i obre el codi generat amb l'Arduino IDE

---

## ① Instal·lació

| Guia | Què aprendràs | Durada |
|------|---------------|--------|
| [Installa i usa OpenCode](./01-installa-opencode/) | Instal·lar, configurar API key i provar OpenCode | 30 min |

## ② Preparar les eines

Abans de programar plaques, cal tenir les eines de compilació:

| Prompt | Per a què? |
|--------|------------|
| [Instal·lar Arduino CLI](./02-prepara-eines/arduino-cli.md) | Compilar i pujar sketches .ino des de terminal (ESP32 + ESP8266) + drivers USB Windows |
| [Instal·lar ESP-IDF](./02-prepara-eines/esp-idf.md) | Framework Espressif per programar ESP32 a baix nivell |
| [Test d'entorn](./02-prepara-eines/test-entorn.md) | Verifica que tot està correcte (OpenCode, CLI, cores, API key, placa) ✅ |
| [Resolució de problemes](./02-prepara-eines/troubleshooting.md) | Errors comuns: permisos, ports, drivers, compilació 🛠️ |
| [Mosquitto clients](./02-prepara-eines/mosquitto-clients.md) | Eines MQTT per terminal (pub/sub) des de Windows/Linux 📡 |
| [Node.js](./02-prepara-eines/nodejs.md) | Prerequisit per instal·lar OpenCode (npm) 🟢 |

## ③ Programar ESP32 amb Arduino

[Guies per a plaques ESP32](./03-esp32-arduino/) — Blink, WiFi, sensor DHT, MQTT.

## ④ Programar ESP8266 amb Arduino

[Guies per a plaques NodeMCU (ESP8266)](./04-esp8266-arduino/) — Blink, WiFi, sensor DHT, MQTT, pinout.

## ⑤ OpenCode fa exercicis sol

Quan ja sàpigues usar OpenCode, [deixa que ell faci la feina](./05-exercicis-auto/) — completar exercicis, depurar codi.

## ⑥ MicroPython per ESP8266

[Guies per programar amb MicroPython](./06-micropython/) — flashejar firmware i pujar scripts.

---

> **Relació amb el curs:** Els exercicis d'ESP32 els trobareu a [../30-code/](../30-code/).

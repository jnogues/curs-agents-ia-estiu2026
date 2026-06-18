# 💻 Guia d'OpenCode

OpenCode és un agent de programació autònom, de codi obert i gratuït. T'ajuda a escriure, revisar i millorar codi directament des del terminal.

Perfecte per programar **ESP32** i **ESP8266** amb assistència d'IA!

---

## ① Instal·lació

| # | Guia | Què aprendràs | Durada |
|---|------|---------------|--------|
| 01 | [Installa i usa OpenCode](./01-installa-opencode/README.md) | Instal·lar, configurar API key i provar OpenCode | 30 min |

## ② Preparar les eines

Abans de programar plaques, cal tenir les eines de compilació:

| Prompt | Per a què? |
|--------|------------|
| [Instal·lar Arduino CLI](./02-prepara-eines/arduino-cli.md) | Compilar i pujar sketches .ino des de terminal (ESP32 + ESP8266) + drivers USB Windows |
| [Instal·lar ESP-IDF](./02-prepara-eines/esp-idf.md) | Framework Espressif per programar ESP32 a baix nivell |
| [Test d'entorn](./02-prepara-eines/test-entorn.md) | Verifica que tot està correcte (OpenCode, CLI, cores, API key, placa) ✅ |
| [Resolució de problemes](./02-prepara-eines/troubleshooting.md) | Errors comuns: permisos, ports, drivers, compilació 🛠️ |

## ③ Programar ESP32 amb Arduino

**Per a alumnes amb placa ESP32 (ISB-32devBoard o similar):**

| Prompt | Què fa |
|--------|--------|
| [Blink](./03-esp32-arduino/blink.md) | LED parpellejant al GPIO2 |
| [WiFi](./03-esp32-arduino/connecta-wifi.md) | Connectar-se a WiFi i fer petició HTTP |
| [Sensor DHT](./03-esp32-arduino/sensor-dht.md) | Llegir temperatura i humitat |
| [MQTT](./03-esp32-arduino/mqtt.md) | Publicar i subscriure's a MQTT |

## ④ Programar ESP8266 amb Arduino

**Per a alumnes amb placa NodeMCU (ESP8266) — [consulta el pinout](04-esp8266-arduino/hardware.md):**

| Prompt | Què fa |
|--------|--------|
| [Blink](./04-esp8266-arduino/blink.md) | LED integrat al GPIO16 |
| [WiFi](./04-esp8266-arduino/connecta-wifi.md) | Connectar-se a WiFi i fer petició HTTP |
| [Sensor DHT](./04-esp8266-arduino/sensor-dht.md) | Llegir temperatura i humitat |
| [MQTT](./04-esp8266-arduino/mqtt.md) | Publicar i subscriure's a MQTT |

## ⑤ OpenCode fa exercicis sol

Quan ja sàpigues usar OpenCode, deixa que ell faci la feina:

| Prompt | Què fa |
|--------|--------|
| [Completar exercici 01](./05-exercicis-auto/completar-ex01.md) | Llegeix, analitza i millora l'Exercici 01 del curs |
| [Completar qualsevol](./05-exercicis-auto/completar-qualsevol.md) | Tria l'exercici (01-08) que vulguis fer |
| [Depurar codi](./05-exercicis-auto/depurar-codi.md) | Revisa i corregeix errors automàticament |

## ⑥ MicroPython per ESP8266

Per als que vulgueu programar la NodeMCU amb MicroPython enlloc d'Arduino:

| Prompt | Què fa |
|--------|--------|
| [Flashejar MicroPython](./06-micropython/flasheja-micropython.md) | Esborra flash, descarrega firmware i flasheja |
| [Pujar scripts](./06-micropython/puja-scripts.md) | Puja boot.py i exercicis al dispositiu |

> Els exercicis de MicroPython els trobareu a [../35-micropython-esp8266/](../35-micropython-esp8266/).

---

> **Relació amb el curs:** Els exercicis d'ESP32 els trobareu a [../30-code/](../30-code/).

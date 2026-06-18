# Instal·lar Arduino CLI (prompt per a OpenCode)

Copia i enganxa això a OpenCode:

```
Vull instal·lar Arduino CLI al meu ordinador i configurar-lo per programar ESP32 i ESP8266.

Primer detecta si el meu sistema és Windows o Ubuntu/Linux.

A Ubuntu/Linux:

1. Descarrega i instal·la Arduino CLI:
   curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=$HOME/.local/bin sh
   (Si no tens curl, instal·la'l primer amb sudo apt install curl)

2. Assegura't que $HOME/.local/bin està al PATH. Si no, afegeix-ho al teu ~/.bashrc:
   echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
   source ~/.bashrc

A Windows:

1. Prova de instal·lar amb winget:
   winget install Arduino.ArduinoCLI

2. Si winget no funciona, descarrega l'última versió per Windows des de:
   https://github.com/arduino/arduino-cli/releases/latest
   (el fitxer acabat en Windows_64bit.zip), descomprimeix-lo i posa'l a C:\arduino-cli\ i afegeix-lo al PATH.

3. Instal·la els drivers USB del xip sèrie de la teva placa (si Windows no els reconeix):
   - NodeMCU V2 (ESP8266): xip **CH340** — descarrega des de https://www.wch.cn/download/CH341SER_EXE.html
   - ISB-32devBoard (ESP32-S3): xip **CP2102** — normalment Windows Update els instal·la sol, però si no, descarrega des de https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
   Si no saps quin xip tens, mira la serigrafia de la placa al costat del connector USB.

Un cop instal·lat (ambdós sistemes):

3. Configura Arduino CLI:
   arduino-cli config init

4. Afegeix les URLs dels board managers de tercers (per ESP32 i ESP8266):
   arduino-cli config add board_manager.additional_urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   arduino-cli config add board_manager.additional_urls https://arduino.esp8266.com/stable/package_esp8266com_index.json

5. Actualitza l'índex de plaques:
   arduino-cli core update-index

6. Instal·la el suport per ESP32:
   arduino-cli core install esp32:esp32

7. Instal·la el suport per ESP8266:
   arduino-cli core install esp8266:esp8266

Verificació final:

8. Confirma que tot funciona:
   arduino-cli version
   arduino-cli core list
   (Han d'aparèixer esp32:esp32 i esp8266:esp8266 com a instal·lats)

Si algun pas falla, explica'm l'error i intenta una alternativa. Al final, mostra'm un resum de què s'ha instal·lat i com provar-ho amb un blink per a cada placa.
```

> **Versió preliminar** — pendent de reorganitzar amb la resta de la carpeta 25.

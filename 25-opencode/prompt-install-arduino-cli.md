# Instal·lar Arduino CLI (prompt per a OpenCode)

Copia i enganxa això a OpenCode:

```
Vull instal·lar Arduino CLI al meu ordinador i configurar-lo per programar ESP32.

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

Un cop instal·lat (ambdós sistemes):

3. Configura Arduino CLI:
   arduino-cli config init

4. Actualitza l'índex de plaques:
   arduino-cli core update-index

5. Instal·la el suport per ESP32:
   arduino-cli core install esp32:esp32

Verificació final:

6. Confirma que tot funciona:
   arduino-cli version
   arduino-cli core list

Si algun pas falla, explica'm l'error i intenta una alternativa. Al final, mostra'm un resum de què s'ha instal·lat i com provar-ho amb un blink.
```

> **Versió preliminar** — pendent de reorganitzar amb la resta de la carpeta 25.

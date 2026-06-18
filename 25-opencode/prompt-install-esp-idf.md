# Instal·lar ESP-IDF 6.x (prompt per a OpenCode)

Copia i enganxa això a OpenCode:

```
Vull instal·lar l'ESP-IDF (Espressif IoT Development Framework) versió 6.x al meu ordinador per programar ESP32 a baix nivell.

Primer detecta si el meu sistema és Windows o Ubuntu/Linux.

A Ubuntu/Linux:

1. Instal·la les dependències del sistema:
   sudo apt-get update
   sudo apt-get install -y git wget flex bison gperf python3 python3-pip python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0

2. Crea un directori per a l'ESP-IDF:
   mkdir -p ~/esp
   cd ~/esp

3. Clona el repositori (versió 6.x):
   git clone --recursive https://github.com/espressif/esp-idf.git -b v6.x
   (Si v6.x no existeix, prova release/v6.x o l'última versió estable disponible)

4. Executa l'instal·lador (per ESP32 genèric):
   cd ~/esp/esp-idf
   ./install.sh esp32

5. Configura l'entorn (afegeix-ho al ~/.bashrc per tenir-lo sempre):
   echo 'alias get_idf="source ~/esp/esp-idf/export.sh"' >> ~/.bashrc
   source ~/esp/esp-idf/export.sh

A Windows:

1. Comprova que tens Git instal·lat (git --version). Si no, descarrega'l des de:
   https://git-scm.com/download/win

2. Obre PowerShell com a administrador i executa:
   mkdir C:\esp -Force
   cd C:\esp

3. Clona el repositori:
   git clone --recursive https://github.com/espressif/esp-idf.git -b v6.x
   (Si v6.x no existeix, prova release/v6.x o l'última versió estable)

4. Executa l'instal·lador:
   cd C:\esp\esp-idf
   .\install.bat esp32

5. Un cop acabi, per activar l'entorn:
   .\export.bat

   (O afegeix-ho al PATH de Windows per tenir-ho sempre disponible)

Un cop instal·lat (ambdós sistemes):

6. Verifica que funciona:
   idf.py --version
   (Hauria de mostrar la versió de l'ESP-IDF)

7. Prova de crear un projecte exemple:
   cd ~/esp (o C:\esp)
   idf.py create-project test-project
   cd test-project
   idf.py set-target esp32
   idf.py build

Si algun pas falla, explica'm l'error i intenta una alternativa. Al final, mostra'm un resum de què s'ha instal·lat i com activar l'entorn la propera vegada.
```

> **Versió preliminar** — pendent de reorganitzar amb la resta de prompts.

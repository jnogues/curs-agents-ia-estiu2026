# Flashejar MicroPython (ESP8266) — prompt per a OpenCode

Copia i enganxa això a OpenCode:

```
Vull flashejar una placa NodeMCU V2 (ESP8266) amb MicroPython des del meu ordinador.

1. Comprova si tinc esptool i mpremote instal·lats (pip install esptool mpremote si no)
2. Detecta a quin port sèrie està connectada la placa
3. Esborra la flash: esptool.py --port <PORT> erase_flash
4. Descarrega l'últim firmware de MicroPython per ESP8266 des de:
   https://micropython.org/download/ESP8266_GENERIC/
   (el build estàndard, no el de 512K)
5. Flasheja: esptool.py --port <PORT> --baud 460800 write_flash --flash_size=detect 0 <firmware.bin>
6. Verifica: mpremote connect <PORT> exec "import sys; print(sys.implementation)"

Al final, mostra'm un resum de tot el que s'ha fet.
```

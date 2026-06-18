# Flashejar una NodeMCU (ESP8266) amb MicroPython

Guia/prompt perquè un amic pugui repetir el mateix procés, ja sigui manualment
o enganxant-ho directament a Claude Code (o un altre assistent amb accés a
terminal).

## 0. Maquinari necessari

- Placa NodeMCU v2/v3 (xip **ESP8266EX**).
- Cable USB (micro-USB normalment) que porti dades, no només càrrega.
- Driver del xip USB-sèrie instal·lat si cal (sol ser **CH340** o **CP2102**;
  en Linux normalment ja funciona sense fer res).

## 1. Prompt per a un assistent (copia i enganxa)

```
Vull flashejar una placa NodeMCU (ESP8266) amb MicroPython des de Linux.

1. Comprova que tinc `esptool` instal·lat (`pip install esptool` si no hi és).
2. Detecta a quin port sèrie està connectada la placa (normalment
   /dev/ttyUSB0) i confirma que esptool s'hi pot connectar amb
   `esptool.py --port <PORT> chip_id`.
3. Fes `esptool.py --port <PORT> erase_flash` per esborrar la flash.
4. Descarrega l'últim firmware estable de MicroPython per a ESP8266 des de
   https://micropython.org/download/ESP8266_GENERIC/ (el build estàndard,
   no el de 512K/1M tret que la meva placa tingui poca flash).
5. Flasheja'l amb:
   esptool.py --port <PORT> --baud 460800 write_flash --flash_size=detect 0 <firmware.bin>
6. Verifica que ha funcionat instal·lant `mpremote` (`pip install mpremote`)
   i executant:
   mpremote connect <PORT> exec "import sys; print(sys.implementation)"
   Hauria de mostrar name='micropython'.

Vull que ho facis tu pas a pas i m'ensenyis la sortida de cada ordre.
```

## 2. Passos manuals (si ho vols fer tu mateix sense assistent)

### 2.1 Instal·lar les eines

```bash
pip install esptool mpremote
```

### 2.2 Trobar el port sèrie

```bash
ls /dev/ttyUSB*      # Linux — sol ser /dev/ttyUSB0
# macOS: ls /dev/tty.usb*
# Windows: mira el Gestor de dispositius (COM3, COM4...)
```

### 2.3 Comprovar que esptool detecta la placa

```bash
esptool.py --port /dev/ttyUSB0 chip_id
```

Hauria de respondre `Chip type: ESP8266EX` i una MAC. Si no respon:

- Prova un altre cable USB (molts només carreguen, no transmeten dades).
- Alguns clons necessiten mantenir premut el botó **FLASH** mentre connectes
  el cable, o just abans d'executar l'ordre.
- Comprova permisos del port a Linux: `sudo usermod -aG dialout $USER` i
  torna a iniciar sessió.

### 2.4 Esborrar la flash

```bash
esptool.py --port /dev/ttyUSB0 erase_flash
```

### 2.5 Descarregar el firmware de MicroPython

Vés a https://micropython.org/download/ESP8266_GENERIC/ i baixa l'última
versió estable (`.bin`). Exemple:

```bash
curl -L -o firmware.bin https://micropython.org/resources/firmware/ESP8266_GENERIC-<versio>.bin
```

### 2.6 Flashejar

```bash
esptool.py --port /dev/ttyUSB0 --baud 460800 write_flash --flash_size=detect 0 firmware.bin
```

### 2.7 Verificar

```bash
mpremote connect /dev/ttyUSB0 exec "import sys; print(sys.implementation)"
```

Si surt `name='micropython'`, ja està. Pots entrar a la REPL interactiva amb:

```bash
mpremote connect /dev/ttyUSB0
```

## 3. Gestió d'scripts

### 3.1 `boot.py` — evitar que `main.py` s'executi

Per poder entrar al REPL encara que hi hagi un `main.py` en marxa:

```python
# boot.py
import gc, machine
gc.collect()

# Si GPIO0 està a GND durant el boot → NO executa main.py
if machine.Pin(0, machine.Pin.IN, machine.Pin.PULL_UP).value() == 0:
    import sys
    sys.exit()
```

- **GPIO0 desconnectat** → `main.py` s'executa normalment
- **GPIO0 a GND + RST** → entra al REPL directament

### 3.2 Pujar scripts

Si el dispositiu **ja està al REPL** (no hi ha `main.py` bloquejant):

```bash
mpremote connect /dev/ttyUSB0 fs cp boot.py :boot.py
mpremote connect /dev/ttyUSB0 fs cp exercici00.py :main.py
mpremote connect /dev/ttyUSB0 reset
```

Si el dispositiu **està bloquejat** per un `main.py` en marxa:

1. Connecta **GPIO0 a GND** i prem **RST** (el `boot.py` amb el nostre trick
   detecta GPIO0=0 i no executa `main.py`)
2. Puja els fitxers amb `mpremote connect`

Alternativa: usar `esptool --port /dev/ttyUSB0 chip-id` per fer un hard reset,
esperar que `main.py` acabi (o que entri al REPL), i després enviar codi via
serial:

```python
import serial, time
s = serial.Serial('/dev/ttyUSB0', 115200, timeout=3)
s.setDTR(False); s.setRTS(False)  # no reset
time.sleep(0.3)
s.write(b"f=open('main.py','w');f.write('# Debug\\n');f.close()\\r\\n")
time.sleep(0.5); print(s.read(4096).decode()); s.close()
```

### 3.3 Llegir memòria

Des del REPL normal:
```python
import gc; gc.collect(); print('Lliure:', gc.mem_free(), 'bytes')
```

O amb mpremote (si el dispositiu no està bloquejat):
```bash
mpremote connect /dev/ttyUSB0 exec "import gc; gc.collect(); print('Lliure:', gc.mem_free(), 'bytes')"
```

### 3.4 Copiar scripts del dispositiu al PC

```bash
mpremote connect /dev/ttyUSB0 fs cp :boot.py boot.py
mpremote connect /dev/ttyUSB0 fs cp :main.py main.py
```

## 4. Notes per a l'assistent (opencode / Claude Code)

- **No manipulis DTR/RTS manualment** amb scripts Python entre el flash i la
  verificació. El CP2102 es desconnecta de l'USB i causa un boot loop.
- Després de flashejar, usa `mpremote` directament (no `serial.Serial`).
- Per esborrar i flashejar, usa sempre `erase_flash` / `write_flash` (sintaxi
  antiga) sobre el port **sense tocar res més**.
- La comanda que ha funcionat:
  ```bash
  esptool.py --port /dev/ttyUSB0 --baud 460800 write_flash --flash_size=detect 0 firmware.bin
  ```

### 4.1 Seqüència de connexió segura (quan el port està "morts")

Quan mpremote no respon (port obert però silenci), fer:

1. `esptool --port /dev/ttyUSB0 chip-id` — fa hard reset via RTS
2. Esperar 3s (que arrenqui MicroPython i acabi `main.py` si n'hi ha)
3. Obrir el port amb DTR/RTS a FALSE per no tornar a resetar:
   ```python
   import serial, time
   ser = serial.Serial('/dev/ttyUSB0', 115200)
   ser.setDTR(False); ser.setRTS(False)
   time.sleep(0.2)
   ser.close()
   ```
4. Ara `mpremote resume exec "..."` hauria de funcionar (entra al raw REPL
   sense fer soft reset).

## 5. Problemes habituals

| Símptoma | Causa probable |
|---|---|
| esptool no troba el port / timeout | Cable només de càrrega, driver USB-sèrie no instal·lat, o placa no entra en mode bootloader (cal prémer FLASH) |
| `Failed to connect` repetit | Baud rate massa alt per al cable/placa; prova `--baud 115200` |
| Flasheja bé però `mpremote` no respon | Espera 1-2 segons després del reset abans de connectar |
| Boot loop (resets continus, dades binàries al port) | **No manipular DTR/RTS amb scripts Python** entre flash i verificació; el CP2102 es desconnecta de l'USB |
| Vols usar GPIO48 / neopixel com a l'ESP32-S3 | No existeix a l'ESP8266 (només arriba a GPIO16); el LED blau integrat sol ser el GPIO2 |

## 6. Notes tècniques

- **MicroPython ESP8266 I2C**: no accepta bus ID. Usar `I2C(scl=Pin(5), sda=Pin(4))`.
- **GPIO3 (RX)**: no usar com a sortida — interfereix amb la comunicació sèrie.
- **Aturar scripts**: Ctrl+C des de Thonny o `mpremote connect ... exec`; si no respon, connectar GPIO0 a GND i RST.

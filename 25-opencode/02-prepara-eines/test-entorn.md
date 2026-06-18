# Test d'entorn (prompt per a OpenCode)

Copia i enganxa això a OpenCode:

```
Vull verificar que el meu entorn de desenvolupament per ESP32/ESP8266 està correcte.

Fes les següents comprovacions:

1. **OpenCode** — confirma que s'ha executat (ja ho estem fent)
2. **Node.js** — `node --version` (cal 18+)
3. **Arduino CLI** — `arduino-cli version`
4. **Cores instal·lats** — `arduino-cli core list` (ha de mostrar esp32:esp32 i esp8266:esp8266)
5. **API key** — comprova que OPENROUTER_API_KEY (o ANTHROPIC_API_KEY) està definida
6. **Port USB** — detecta si hi ha una placa connectada:
   - A Linux: `ls /dev/ttyUSB*` o `ls /dev/ttyACM*`
   - A Windows: `wmic path Win32_SerialPort` o mira els ports COM

Al final, mostra'm un resum tipus:

| Comprovació | Estat |
|-------------|-------|
| OpenCode | ✅ |
| Node.js | ✅ v20.x |
| Arduino CLI | ✅ v1.x |
| Core ESP32 | ✅ |
| Core ESP8266 | ✅ |
| API key | ✅ OPENROUTER_API_KEY |
| Placa connectada | ✅ /dev/ttyUSB0 |

Si alguna cosa falla, explica'm com solucionar-ho.
```

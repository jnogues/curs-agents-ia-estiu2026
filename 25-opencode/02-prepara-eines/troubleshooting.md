# Resolució de problemes (prompt per a OpenCode)

Copia i enganxa això a OpenCode:

```
Tinc problemes amb el meu entorn de desenvolupament ESP32/ESP8266.

Detecta primer el meu sistema (Windows/Linux) i ajuda'm a resoldre:

**Problemes comuns:**

1. **"arduino-cli: command not found"**
   - Linux: comprova que $HOME/.local/bin està al PATH
   - Windows: comprova que la carpeta d'Arduino CLI està al PATH del sistema

2. **No troba el port USB / "No such file or directory"**
   - A Windows: comprova que tens els drivers CH340 o CP2102 instal·lats
   - A Linux: prova `sudo usermod -aG dialout $USER` i reinicia sessió
   - Prova un altre cable USB (no tots porten dades)
   - A l'Arduino IDE: Tools → Port, prova tots els ports disponibles

3. **Error de compilació "esp32:esp32:esp32-s3 not found"**
   - Executa: `arduino-cli core update-index`
   - Després: `arduino-cli core install esp32:esp32`

4. **Error a l'upload "Failed to connect"**
   - Mantén premut el botó BOOT/RST de la placa mentre puja
   - Prova amb un baud rate més baix
   - Assegura't que tens la placa correcta seleccionada

5. **Error de permisos npm a Windows**
   - Obre el terminal com a administrador
   - Prova: `npm install -g opencode-ai@latest`

6. **OpenCode no entén la meva placa**
   - Sigues explícit al prompt: "per a ESP32-S3 amb Arduino framework" o "per a NodeMCU V2 (ESP8266)"

Digues-me quin problema tens i el resolc pas a pas.
```

# Instal·lar Mosquitto clients (prompt per a OpenCode)

Copia i enganxa això a OpenCode:

```
Vull instal·lar les eines de terminal de Mosquitto (mosquitto_pub i mosquitto_sub)
per provar MQTT des del terminal.

Primer detecta si el meu sistema és Windows o Ubuntu/Linux.

A Ubuntu/Linux:

1. Instal·la mosquitto-clients:
   sudo apt update
   sudo apt install -y mosquitto-clients

2. Verifica:
   mosquitto_pub --help
   (Ha de mostrar l'ajuda)

A Windows:

1. Descarrega l'instal·lador des de https://mosquitto.org/download/
   (escull la versió per Windows, per exemple mosquitto-2.x.x-install-windows-x64.exe)

2. Executa l'instal·lador i segueix els passos (següent, següent...)

3. Assegura't que la carpeta d'instal·lació (normalment C:\Program Files\mosquitto)
   està al PATH del sistema. Si no, afegeix-la:

   PowerShell (com a admin):
   [Environment]::SetEnvironmentVariable("Path", "$env:Path;C:\Program Files\mosquitto", "Machine")

4. Verifica obrint un terminal nou:
   mosquitto_pub --help

Un cop instal·lat (ambdós sistemes):

Prova de connectar-te al broker del curs:

- Per publicar:
  mosquitto_pub -h <IP_BROKER> -t "test" -m "Hola des de OpenCode!"

- Per subscriure't:
  mosquitto_sub -h <IP_BROKER> -t "test" -v

Si algun pas falla, explica'm l'error i intenta una alternativa.
```

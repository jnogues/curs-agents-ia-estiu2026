# Code — Exercicis del curs

Índex dels exercicis pràctics d'Arduino IDE per al curs d'agents d'IA.

---

## 📋 Llista d'exercicis

| Exercici | Descripció | Novetat principal | Material extra |
|----------|------------|-------------------|----------------|
| [Exercici-01](Exercici-01/) | Template base: WiFi multi-xarxa, MQTT, Neopixel, temperatura interna | Base | — |
| [Exercici-02](Exercici-02/) | Sensor extern DS18B20 | + DS18B20 (4 tasques FreeRTOS) | DS18B20 + resistència 4,7 kΩ |
| [Exercici-03](Exercici-03/) | Control de 4 LEDs per MQTT (ON/OFF/TOG, retained) | + LEDs per MQTT | 4 LEDs + 4 × 220 Ω |
| [Exercici-04](Exercici-04/) | Telemetria unificada: temperatures i SSID dins del JSON d'estat | + JSON autosuficient | — |
| [Exercici-05](Exercici-05/) | Hora real NTP, parsing JSON robust (ArduinoJson) i mutex | + NTP + ArduinoJson + mutex | — |
| [Exercici-06](Exercici-06/) | Presència MQTT amb LWT (Last Will and Testament) | + LWT `/online` retained | — |
| [Exercici-07](Exercici-07/) | 4 polsadors amb debounce + actuadors TRIAC, Relé i Buzzer | + Polsadors + actuadors (5 tasques) | 4 polsadors |
| [Exercici-08](Exercici-08/) | Polsadors avançats: pulsació curta / llarga → accions locals | + Detecció curt/llarg | — |

---

## 🔧 Estructura comuna

Tots els exercicis comparteixen:

- **Board:** `ESP32S3 Dev Module` (Arduino IDE)
- **Broker MQTT:** `46.224.116.35:1883`
- **LED integrat:** Neopixel a GPIO 48 (WS2812B)
- **Tasques FreeRTOS** al Core 1

Cada alumne ha de canviar `ALUMNE_ID` al seu codi perquè els tòpics MQTT siguin únics.

---

## 📡 Com monitoritzar

```bash
# Veure TOTS els dispositius en directe
mosquitto_sub -h 46.224.116.35 -t "/esp32s3/#" -v

# Veure només un alumne concret
mosquitto_sub -h 46.224.116.35 -t "/esp32s3/alumneXX/#" -v
```

---

## 🧰 Llibreries acumulades

| Exercici | Llibreries necessàries |
|----------|------------------------|
| 01 | MycilaMQTT · Adafruit NeoPixel |
| 02 | + OneWire · DallasTemperature |
| 03 | (les mateixes que 02) |
| 04 | (les mateixes que 03) |
| 05 | + ArduinoJson |
| 06–08 | (les mateixes que 05) |

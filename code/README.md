# Code — Exercicis del curs

Índex dels exercicis pràctics d'Arduino IDE per al curs d'agents d'IA.

---

## 📋 Llista d'exercicis

| Exercici | Descripció | Tòpic MQTT | Material |
|----------|-----------|------------|----------|
| [Exercici-01](Exercici-01/) | Template base: WiFi multi-xarxa, MQTT, Neopixel, temperatura interna | `/esp32s3/<id>/` | ESP32-S3 (només la placa) |
| [Exercici-02](Exercici-02/) | Sensor extern DS18B20: 4 tasques FreeRTOS, temperatura ambient | `/esp32s3/<id>/temperaturaDS` | + DS18B20 + resistència 4,7 kΩ |
| _En breu..._ | | | |

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
mosquitto_sub -h 46.224.116.35 -t "/esp32s3/alumne01/#" -v
```

---

## 🧰 Llibreries acumulades

| Exercici | Llibreries |
|----------|-----------|
| 01 | MycilaMQTT, Adafruit NeoPixel |
| 02 | + OneWire, DallasTemperature |

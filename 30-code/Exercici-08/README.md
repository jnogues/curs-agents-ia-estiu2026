# 08 — Polsadors avançats: pulsació curta i llarga

Evolució de l'Exercici-07: els polsadors ara detecten **pulsació curta** i
**pulsació llarga** (≥ 1 s), i executen accions **locals** directament sense
passar pel broker MQTT.

---

## 🆕 Novetats respecte a l'Exercici-07

| Concepte | Exercici-07 | Exercici-08 |
|----------|-------------|-------------|
| Tipus de pulsació | Premut / Alliberat | **Curta** (< 1 s) i **Llarga** (≥ 1 s) |
| Acció local | — | ✅ Toggle LED directament des del polsador |
| Pols1 curt | Només event MQTT | **Toggle LED1** |
| Pols2 curt | Només event MQTT | **Toggle LED2** |
| Pols2 llarg | Només event MQTT | **Toggle LED3** |
| Pols3 / Pols4 | Només event MQTT | Igual (sense acció local) |
| Stack `polsadorsTask` | 2 KB | **4 KB** (més lògica) |
| Hardware nou | — | Cap |
| Llibreries noves | — | Cap |

---

## ⏱️ Lògica de pulsació curta / llarga

```
Pulsació CURTA (< 1000 ms):
   Prem ────────────────── Allibera
         < 1 s
         → onShortPress() s'executa en alliberar

Pulsació LLARGA (≥ 1000 ms):
   Prem ─────────────────────────────── Allibera
                    ≥ 1 s
         → onLongPress() s'executa mentre es manté premut
           (no espera l'alliberament)
```

> 💡 La pulsació llarga **s'executa immediatament** en arribar al llindar,
> sense esperar que el botó s'alliberi. Quan s'allibera, `onShortPress`
> **no s'executa** (el flag `polsadorLongFired` ho evita).

---

## 🎮 Assignació de polsadors (configurable)

| Polsador | Pulsació curta | Pulsació llarga |
|----------|---------------|-----------------|
| Pols 1 (GPIO 39) | Toggle LED 1 | — |
| Pols 2 (GPIO 40) | Toggle LED 2 | Toggle LED 3 |
| Pols 3 (GPIO 41) | Només event MQTT | — |
| Pols 4 (GPIO 42) | Només event MQTT | — |

Per canviar les assignacions, edita les funcions `onShortPress()` i
`onLongPress()` al codi:

```cpp
void onShortPress(int boto) {
  switch (boto) {
    case 1: toggleLedLocal(0); break;  // Polsador 1 → LED 1
    case 2: toggleLedLocal(1); break;  // Polsador 2 → LED 2
    default: break;
  }
}

void onLongPress(int boto) {
  switch (boto) {
    case 2: toggleLedLocal(2); break;  // Polsador 2 llarg → LED 3
    default: break;
  }
}
```

---

## 📡 Tòpics MQTT

Iguals que l'Exercici-07 — els polsadors **segueixen publicant** events
`PREMUT` i `ALLIBERAT` independentment de les accions locals:

| Tòpic | Direcció | Retained | Contingut |
|-------|----------|----------|-----------|
| `/esp32s3/<id>/estat` | ESP32 → Broker | No | JSON complet cada 5 s |
| `/esp32s3/<id>/stateLEDS` | ESP32 → Broker | **Sí** ✅ | `{"leds":[…],"triac":…,"rele":…}` |
| `/esp32s3/<id>/ordre` | Broker → ESP32 | No | Comandes LEDs + actuadors |
| `/esp32s3/<id>/online` | Broker → tots | **Sí** ✅ | `"1"` / `"0"` LWT |
| `/esp32s3/<id>/polsadors` | ESP32 → Broker | No | `{"boto":1,"event":"PREMUT"}` |

---

## ⚙️ Arquitectura (5 tasques — sense canvis de nombre)

```
Core 1
├── neopixelTask    (prio 3 · 3 KB)
├── polsadorsTask   (prio 3 · 4 KB)  ← Detecta curt/llarg + acció local ⭐
├── tempInternaTask (prio 4 · 4 KB)
├── tempDS18B20Task (prio 4 · 4 KB)
└── mainTask        (prio 5 · 8 KB)
```

### Flux de `polsadorsTask` (simplificat)

```
cada 50 ms per a cada polsador:
  ├── Flanc detectat? → iniciar debounce
  ├── Debounce confirmat i LOW? → desar temps, publicar PREMUT
  ├── Debounce confirmat i HIGH? → si no longFired → onShortPress()
  │                                publicar ALLIBERAT
  └── Premut des de fa ≥ 1000 ms i no longFired? → onLongPress(), longFired=true
```

---

## 🐛 Troubleshooting

| Símptoma | Causa | Solució |
|----------|-------|---------|
| La pulsació llarga no es detecta | Llindar massa alt | Redueix `LONG_PRESS_MS` (prova 500) |
| Curt i llarg s'activen alhora | `polsadorLongFired` no s'elimina | Reinicia; si persisteix, comprova el debounce |
| LED no canvia en prémer | `toggleLedLocal` no agafa el mutex | Comprova que `dataMutex` es crea a `setup()` |
| Events MQTT duplicats | Rebots del polsador | Augmenta `DEBOUNCE_MS` |

---

## 📁 Fitxers

| Fitxer | Descripció |
|--------|------------|
| `Exercici-08.ino` | Codi font amb polsadors avançats (curt/llarg) |
| `README.md` | Aquest document |

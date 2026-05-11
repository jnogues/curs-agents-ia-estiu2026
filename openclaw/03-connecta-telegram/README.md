# 📱 Activitat 03: Connecta OpenClaw a Telegram

**Durada:** ~15 minuts
**Dificultat:** ⭐⭐ (una mica més avançada)
**Requisit:** Haver completat la [Guia 01](../01-installa-openclaw-vps/README.md)

## Objectiu

Parlar amb el teu OpenClaw des del Telegram del mòbil! 🎉

---

## Pas 1: Crea un bot de Telegram

Obre Telegram i busca **@BotFather**:

```
/newbot
```

Et demanarà dos noms:
- **Nom del bot:** posa el que vulguis (ex: `OpenClaw_Berta`)
- **Username:** ha d'acabar en `bot` (ex: `OpenClawBertaBot`)

El BotFather et donarà un **token**:
```
1234567890:ABCdefGHIjklmNOPqrstUVwxyz
```

> 🔑 **Guarda'l!** El necessitaràs al pas 2.

---

## Pas 2: Configura el bot a OpenClaw

Tens dues opcions:

### Opció A: Via wizard (recomanat)

Torna a executar el wizard de configuració i digues que **sí** a canals:

```bash
openclaw onboard
```

Quan et pregunti *"Do you want to set up messaging channels?"*, respon que **sí**.
Tria **Telegram** i enganxa el token del BotFather.

### Opció B: Via variable d'entorn (més ràpid)

```bash
export TELEGRAM_BOT_TOKEN=EL_TOKEN_DEL_TEU_BOT
```

(Si poses aquesta línia a `~/.bashrc`, no caldrà tornar-la a escriure mai més)

---

## Pas 3: Reinicia el gateway

Perquè els canvis tinguin efecte:

```bash
openclaw gateway restart
```

Comprova que el gateway està actiu amb Telegram connectat:

```bash
openclaw gateway status
```

Hauries de veure alguna cosa com:
```
telegram: connected ✓
```

---

## Pas 4: Pareja el bot amb tu

Obre Telegram, busca el teu bot (ex: `@OpenClawBertaBot`) i envia-li:

```
Hola!
```

El bot et respondrà amb un **codi de parell** (ex: `VV5RXRDD`).

Al VPS, aprova'l:

```bash
openclaw pairing list
openclaw pairing approve telegram VV5RXRDD
```

*(Substitueix `VV5RXRDD` pel codi que t'hagi donat el bot)*

---

## Pas 5: Prova final! 🎉

Des del mòbil, envia-li al teu bot:

```
Hola! Què saps fer?
```

Si tot va bé, OpenClaw et respondrà al mòbil! 📱

---

## ✨ Què has après?

- Crear un bot de Telegram amb @BotFather
- Configurar Telegram a OpenClaw (wizard o variable d'entorn)
- Com funciona el sistema de parell (pairing)
- Que OpenClaw viu tant al servidor com al teu mòbil!

## 💡 Per anar més lluny

- Envia-li **fotos o arxius** des del mòbil
- Prova d'enviar-li **missatges de veu** (si el profe ha configurat STT)
- Configura'l perquè **t'enviï el temps cada matí** (cron jobs — pregunta al profe)

## ❌ No funciona?

| Problema | Solució |
|----------|---------|
| **El bot no respon** | `openclaw gateway status` — Telegram ha de dir "connected" |
| **"no autoritzat"** | Torna a fer el pairing: `openclaw pairing list` + `approve` |
| **Token incorrecte** | Torna a configurar-lo amb `openclaw onboard` o `export TELEGRAM_BOT_TOKEN=...` |
| **El gateway no s'inicia** | `openclaw gateway restart` i després `openclaw gateway status` |
| **Error al fer pairing** | Assegura't que primer has enviat "Hola" des de Telegram |
| **Res de res** | Aviseu el profe! 😅 |

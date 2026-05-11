# 📱 Activitat 05: Connecta Hermes al teu mòbil (Telegram)

**Durada:** ~15 minuts
**Dificultat:** ⭐⭐ (una mica més avançada)

## Objectiu

Parlar amb el teu Hermes des del Telegram del mòbil! 🎉

## Passos

### 1. Crea un bot de Telegram

Obre Telegram i busca **@BotFather** (és el bot oficial de Telegram per crear bots).

Escriu-li:

```
/newbot
```

Et demanarà dos noms:
- **Nom del bot:** `Hermes [El teu nom]` (ex: `Hermes Berta`)
- **Username del bot:** Ha d'acabar en `bot`. Ex: `HermesBertaBot`

El BotFather et donarà un **token** que sembla això:
```
1234567890:ABCdefGHIjklmNOPqrstUVwxyz
```

**Guarda aquest token!** El necessitaràs al pas 2.

### 2. Configura el gateway al VPS

Des del terminal (connectat al VPS):

```bash
export PATH="$HOME/.local/bin:$PATH"
hermes gateway setup
```

Selecciona **Telegram** de la llista de plataformes.
Quan et demani el **token**, enganxa el que t'ha donat el BotFather.

### 3. Instal·la el servei

```bash
hermes gateway install
systemctl --user start hermes-gateway
systemctl --user status hermes-gateway    # Ha de dir "active"
```

### 4. Pareja el bot amb tu

Obre Telegram, busca el teu bot pel username (ex: `@HermesBertaBot`) i envia-li:

```
Hola!
```

El bot et respondrà amb un **codi de parell** (ex: `VV5RXRDD`).

Ara, al terminal del VPS:

```bash
hermes pairing list
hermes pairing approve telegram VV5RXRDD
```

*(Substitueix `VV5RXRDD` pel teu codi)*

### 5. Prova final! 🎉

Des del mòbil, envia-li al teu bot:

```
Quin temps fa?
```

Si tot funciona, Hermes et respondrà al mòbil!

## ✨ Què has après?

- Com crear un bot de Telegram amb @BotFather
- Com configurar el gateway d'Hermes per xatejar des del mòbil
- Com funciona el sistema de parell (pairing) per seguretat
- Que Hermes no viu només al terminal — pot estar al teu mòbil!

## 💡 Per anar més lluny

- Prova d'enviar-li **fotos o àudios** des del mòbil
- Demana-li que **et recordi coses** (la memòria funciona també des de Telegram)
- Configura'l perquè **s'enviï notícies cada matí** (parla amb el profe sobre cron jobs)

## ❌ No funciona?

- **El bot no respon:** Assegura't que el gateway està actiu: `systemctl --user status hermes-gateway`
- **El bot diu "no autoritzat":** Has fet el pairing? Prova `hermes pairing list` i torna a aprovar
- **Error al configurar:** Prova `hermes gateway setup` de nou
- **Error de token:** Assegura't que el token del BotFather està ben copiat (sense espais)
- **El gateway no s'inicia:** Reinicia'l: `systemctl --user restart hermes-gateway`
- **No funciona res:** Aviseu el profe!

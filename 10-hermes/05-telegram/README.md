# 📱 Activitat 05: Connecta Hermes al teu mòbil (Telegram)

|**Durada:** ~15 minuts
**Dificultat:** ⭐⭐ (una mica més avançada)

## Objectiu

Parlar amb el teu Hermes des del Telegram del mòbil! 📱✨

## Passos

### 1. Crea un bot de Telegram

Obre Telegram i busca **@BotFather**:

```
/newbot
```

Et demanarà dos noms:
- **Nom del bot:** posa el que vulguis (ex: `Hermes_Berta`)
- **Username del bot:** ha d'acabar en `bot` (ex: `HermesBertaBot`)

El BotFather et donarà un **token**:
```
1234567890:ABCdefGHIjklmNOPqrstUVwxyz
```

> 🔑 **Guarda'l!** El necessitaràs al pas 2.

---

### 2. Configura el gateway

Des del terminal del VPS:

```bash
export PATH="$HOME/.local/bin:$PATH"
hermes gateway setup
```

L'assistent et guiarà:
1. Selecciona **Telegram** de la llista
2. Enganxa el **token** del BotFather

---

### 3. Instal·la el servei

```bash
hermes gateway install
systemctl --user start hermes-gateway
systemctl --user status hermes-gateway    # Ha de dir "active"
```

> ⚠️ **Perquè no es mori en tancar la sessió SSH:**
> ```bash
> sudo loginctl enable-linger $USER
> ```

---

### 4. Pareja el bot amb tu

Obre Telegram, busca el teu bot (ex: `@HermesBertaBot`) i envia-li:

```
Hola!
```

El bot et respondrà amb un **codi** (ex: `VV5RXRDD`).

Al VPS:

```bash
hermes pairing list
hermes pairing approve telegram VV5RXRDD
```

*(Substitueix `VV5RXRDD` pel teu codi)*

---

### 5. Prova final! 🎉

Des del mòbil, envia-li:

```
Quin temps fa?
```

Si tot va bé, Hermes et respondrà al mòbil! 📱

---

## ✨ Què has après?

- Crear un bot de Telegram amb @BotFather
- Configurar el gateway d'Hermes al VPS
- Com funciona el sistema de parell (pairing)
- Que Hermes també viu al mòbil!

## 💡 Per anar més lluny

- Envia-li **fotos o àudios** des del mòbil
- Pregunta-li coses que va aprendre a l'Activitat 04 (la memòria també funciona des de Telegram!)
- Demana al profe com configurar **notícies automàtiques cada matí** (cron jobs)

## ❌ No funciona?

| Problema | Solució |
|----------|---------|
| **El bot no respon** | `systemctl --user status hermes-gateway` — ha de dir "active" |
| **"no autoritzat"** | Torna a fer el pairing: `hermes pairing list` + `approve` |
| **Error al configurar** | Prova `hermes gateway setup` de nou |
| **Error de token** | Assegura't que està ben copiat (sense espais) |
| **El gateway no s'inicia** | `systemctl --user restart hermes-gateway` |
| **Es mor en sortir del SSH** | Has fet `sudo loginctl enable-linger $USER`? |
| **Res de res** | Aviseu el profe! 😅 |

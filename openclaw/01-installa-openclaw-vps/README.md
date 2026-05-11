# 🦞 Guia 01: Installa OpenClaw en un VPS

**Durada:** ~30-45 minuts
**Dificultat:** ⭐⭐ (cal saber una mica de terminal)
**Cost:** ~4€/mes (el VPS)

## Objectiu

Tenir el teu propi OpenClaw Agent en un servidor al núvol, funcionant 24/7! 🚀

## Què necessites?

- Un VPS (consulta l'[Activitat 06 d'Hermes](../../hermes/06-compra-un-vps/README.md) per saber com comprar-ne un)
- Saber copiar i enganxar comandes al terminal
- Una API key d'Anthropic (Claude), OpenAI (GPT) o Google (Gemini)

---

## Pas 1: Connecta't al VPS

```bash
ssh root@<IP_DEL_VPS>
```

*(Si és el teu primer cop, et demanarà la contrasenya que has rebut per email)*

> **Primera vegada?** Escriu `yes` per confirmar la connexió.

---

## Pas 2: Installa Node.js 22

OpenClaw funciona amb Node.js. Necessites la versió 22 o superior.

```bash
# Descarrega el script d'instal·lació de NodeSource
curl -fsSL https://deb.nodesource.com/setup_22.x -o nodesetup.sh

# Executa'l
bash nodesetup.sh

# Installa Node.js
apt install -y nodejs

# Verifica
node --version   # Ha de mostrar v22.x.x
npm --version    # Ha de mostrar 10.x o superior
```

---

## Pas 3: Installa OpenClaw

```bash
npm install -g openclaw
```

(Tarda 1-2 minuts)

Verifica:

```bash
openclaw --version
```

---

## Pas 4: Abans de configurar, aconsegueix una API key

OpenClaw necessita un model d'IA per funcionar. Tria el que vulguis:

### Opció A: Anthropic Claude (Recomanat, millor qualitat)

1. Ves a [console.anthropic.com](https://console.anthropic.com) i crea un compte
2. Ves a **API Keys** → **Create Key**
3. Copia la clau (comença per `sk-ant-...`)

**Cost estimat:** ~1-2€/mes amb ús moderat

### Opció B: OpenAI GPT

1. Ves a [platform.openai.com/api-keys](https://platform.openai.com/api-keys)
2. Crea una API key
3. Copia-la

**Cost estimat:** ~1-2€/mes

### Opció C: Google Gemini (Gratis)

1. Ves a [aistudio.google.com/apikey](https://aistudio.google.com/apikey)
2. Clica **Create API Key**
3. Copia la clau (comença per `AIza...`)

**Cost:** 0€ (60 req/min, suficient per a ús personal)

---

## Pas 5: Configura OpenClaw

Executa l'assistent de configuració:

```bash
openclaw onboard
```

L'assistent et guiarà pas a pas per:

1. **Workspace** — On desar els fitxers (prem Enter per acceptar `~/.openclaw/`)
2. **API keys** — Les que has aconseguit al Pas 4
3. **Plataformes** — Pots connectar Telegram, Discord, etc. (opcional)
4. **Skills** — Pots instal·lar-ne algunes bàsiques

> **Consell:** Si no saps què posar en algun pas, prem **Enter** per saltar-lo. Ho pots configurar després.

---

## Pas 6: Prova'l

```bash
openclaw chat -q "Hola! Explica'm què ets i què saps fer"
```

Si tot funciona, veuràs la resposta! 🎉

---

## Pas 7: Installa com a servei (per 24/7)

Perquè OpenClaw funcioni sempre, l'has de deixar corrent en segon pla:

```bash
openclaw onboard --install-daemon
```

Això crea un servei systemd que:
- S'engega automàticament en reiniciar el VPS
- Es reinicia si falla
- Corre en segon pla

Comprova que funciona:

```bash
systemctl status openclaw-gateway
```

> ⚠️ **Important:** Perquè el servei sobrevisqui al tancament de la sessió SSH:
> ```bash
> sudo loginctl enable-linger root
> ```

### Alternativa amb PM2

Si prefereixes PM2 (gestor de processos Node.js):

```bash
npm install -g pm2
pm2 start openclaw -- start
pm2 save
pm2 startup
```

---

## Pas 8: (Opcional) Connecta Telegram

Parla amb OpenClaw des del mòbil:

1. Obre Telegram i busca **@BotFather**
2. Envia-li `/newbot` i segueix les instruccions
3. Guarda el **token** que et doni

Edita el fitxer de configuració:

```bash
nano ~/.openclaw/openclaw.json
```

Afegeix-hi:

```json
{
  "channels": {
    "telegram": {
      "enabled": true,
      "token": "EL_TOKEN_DEL_TEU_BOT"
    }
  }
}
```

Reinicia el servei:

```bash
systemctl restart openclaw-gateway
```

Ara busca el teu bot a Telegram i envia-li "Hola"! 📱

---

## 🎯 Llista de verificació

- [ ] VPS comprat i funcionant
- [ ] Connectat per SSH
- [ ] Node.js 22 instal·lat (`node --version`)
- [ ] OpenClaw instal·lat (`openclaw --version`)
- [ ] API Key configurada
- [ ] Respon correctament (`openclaw chat -q "Hola"`)
- [ ] Servei actiu (systemd o PM2)
- [ ] (Opcional) Telegram connectat

---

## 💡 Diferències amb Hermes Agent

| Aspecte | Hermes | OpenClaw |
|---------|--------|----------|
| Llenguatge | Python | Node.js / TypeScript |
| Instal·lació | `curl ... install.sh` | `npm install -g openclaw` |
| Configuració | `hermes config set ...` | `openclaw onboard` |
| Fitxer de config | `~/.hermes/config.yaml` | `~/.openclaw/openclaw.json` |
| Skills | Fitxers SKILL.md | Plugins npm + skills |
| Models | 20+ providers | Anthropic, OpenAI, Gemini, Ollama... |
| Servei | `hermes gateway install` | `openclaw onboard --install-daemon` |
| Cost VPS | ~4€/mes | ~4€/mes |

---

## ❌ Problemes comuns

- **`openclaw: command not found`** → Prova `export PATH="$HOME/.npm-global/bin:$PATH"` o `source ~/.bashrc`
- **`npm install` falla** → Prova amb `sudo npm install -g openclaw`
- **Error d'API Key** → Revisa `~/.openclaw/openclaw.json`
- **El servei no arrenca** → Mira els logs: `journalctl -u openclaw-gateway --no-pager | tail -30`
- **Node.js massa vell** → `node --version` ha de mostrar v22.x
- **Bot de Telegram mut** → El servei està actiu? `systemctl status openclaw-gateway`

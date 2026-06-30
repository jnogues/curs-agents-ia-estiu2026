# ☁️ Activitat 00b: Instal·la Hermes en un VPS

**Durada:** ~30-45 minuts
**Dificultat:** ⭐⭐⭐ (cal saber una mica de terminal)
**Cost:** ~4€/mes

## Objectiu

Tenir el teu propi Hermes Agent en un servidor al núvol, totalment teu, 24/7! 🚀

## Què necessites?

- Un compte bancari (per pagar el VPS)
- Uns 4-8€ al mes
- Saber copiar i enganxar comandes al terminal

> 💡 **Ja tens Hermes instal·lat al teu PC?** Aquesta activitat és per qui vol un servidor sempre encès. Si només vols provar Hermes, ves a la [instal·lació local](../00a-instalacio-local/README.md).

---

## Pas 1: Compra un VPS

Un VPS (Virtual Private Server) és un ordinador sempre encès al núvol. La millor opció qualitat-preu:

### 🏆 Recomanació: Hetzner CX23 (~4€/mes)

| Característica | Valor |
|---------------|-------|
| RAM | 4 GB |
| CPU | 2 vCPU |
| Disc | 40 GB SSD |
| Preu | ~3.99€/mes |
| Ubicació | Finlàndia o Alemanya |

**Com comprar-lo:**

1. Ves a [hetzner.com](https://www.hetzner.com) → **Cloud**
2. Crea un compte (email, nom, adreça, targeta de crèdit)
3. Un cop dins, ves a **Projects** → **Create Project**
4. Dins del projecte, **Create Server**:
   - **Location:** Nuremberg o Helsinki (el més barat)
   - **Image:** Ubuntu 26.04 LTS
   - **Type:** CX23
   - **SSH Keys:** Opcional (després pots fer servir contrasenya)
5. Clica **Create & Buy**

En 1-2 minuts rebràs un email amb la **IP del servidor** i la **contrasenya root**.

### Alternatives econòmiques

| Proveïdor | Preu mínim | RAM | Bo |
|-----------|:----------:|:---:|:---|
| **Hetzner CX23** | ~4€/mes | 4 GB | Millor relació Q/P |
| **Netcup** | ~3.50€/mes | 4 GB | Similar a Hetzner |
| **Ionos** | ~2€/mes (6 mesos) | 1 GB | Barat per començar |
| **Oracle Cloud Free** | 0€/mes | 1 GB | Gratis, però registre complicat |
| **DigitalOcean** | ~4$/mes | 512 MB | Petitet però bon suport |
| **AWS Lightsail** | ~3.5$/mes | 512 MB | Per usuaris avançats |

> **Consell:** Per començar, Hetzner és el més fàcil i fiable.

---

## Pas 2: Connecta't al VPS (SSH)

Obre un terminal al teu ordinador i escriu:

```bash
ssh root@<IP_DEL_VPS>
```

*(Substitueix `<IP_DEL_VPS>` per la IP que t'ha enviat Hetzner)*

Quan et demani la contrasenya, enganxa la que has rebut per email.

> **Primera vegada?** Et preguntarà si confies en el servidor. Escriu `yes`.

**Ja ets dins del servidor!** 🎉

---

## Pas 3: Instal·la Hermes

Un cop connectat, executa:

```bash
curl -fsSL https://raw.githubusercontent.com/NousResearch/hermes-agent/main/scripts/install.sh | bash
```

Això baixa i instal·la Hermes automàticament. Tarda uns 2-3 minuts.

Quan acabi, actualitza el PATH per poder usar `hermes`:

```bash
source ~/.bashrc
```

Prova que s'ha instal·lat correctament:

```bash
hermes --version
```

---

## Pas 4: Configura una API Key

Hermes necessita un model d'IA per funcionar. Necessites una API key d'algun proveïdor.

### Opció A: DeepSeek (Recomanat — Molt barat)

1. Ves a [platform.deepseek.com](https://platform.deepseek.com) i crea un compte
2. Ves a **API Keys** → **Create API Key**
3. Copia la clau (comença per `sk-...`)

Al teu VPS:

```bash
cat > ~/.hermes/.env << 'EOF'
DEEPSEEK_API_KEY=sk-EL_TEU_TOKEN_AQUI
EOF

export PATH="$HOME/.local/bin:$PATH"
hermes config set model.provider deepseek
hermes config set model.default deepseek/deepseek-chat
```

**(Cost estimat: ~1€ al mes usant-lo cada dia)**

### Opció B: Gemini (Gratis — Però límit de 60 req/min)

1. Ves a [aistudio.google.com/apikey](https://aistudio.google.com/apikey)
2. Clica **Create API Key**
3. Copia la clau (comença per `AIza...`)

```bash
cat > ~/.hermes/.env << 'EOF'
GEMINI_API_KEY=AIzaEL_TEU_TOKEN_AQUI
EOF

export PATH="$HOME/.local/bin:$PATH"
hermes config set model.provider gemini
hermes config set model.default gemini-2.0-flash
```

---

## Pas 5: Prova'l!

```bash
hermes chat -q "Hola! Soc el teu nou propietari. Com estàs?"
```

Si tot funciona, veuràs la resposta d'Hermes! 🎉

Prova més coses:

```bash
hermes chat -q "Explica'm 3 coses que sàpigues fer"
```

---

## ➡️ I ara què?

- **Vols desplegar serveis al teu VPS?** Fes la [Activitat 00c: Docker + Serveis al VPS](../00c-docker-serveis-vps/README.md) per tenir MQTT, InfluxDB, Grafana i Node-RED
- **Vols parlar des del mòbil?** Fes l'[Activitat 05: Telegram](../05-telegram/README.md) per connectar el bot
- **No saps què fer al VPS?** Torna al [menú principal](../README.md)

---

## 🎯 Llista de verificació

- [ ] VPS comprat i funcionant (Hetzner, DigitalOcean...)
- [ ] Connectat per SSH
- [ ] Hermes instal·lat (`hermes --version` funciona)
- [ ] API Key configurada (DeepSeek o Gemini)
- [ ] Model configurat
- [ ] Hermes respon (`hermes chat -q "Hola"`)

## 💡 Consells de seguretat

Un cop tinguis Hermes funcionant, no està de més assegurar el servidor:

```bash
# Canvia la contrasenya root
passwd

# Activa el firewall (permet només SSH)
ufw allow 22
ufw enable

# Mantén el sistema actualitzat
apt update && apt upgrade -y
```

## 🧹 Neteja: Donar de baixa el VPS

Si ja no el vols fer servir, recorda **cancel·lar-lo** des del panell de Hetzner per no pagar del tot. Normalment es pot fer en qualsevol moment.

## ❌ Problemes comuns

- **`hermes: command not found`** → Has d'executar `source ~/.bashrc` o `export PATH="$HOME/.local/bin:$PATH"`
- **Error 401 / no autoritzat** → L'API Key no és correcta. Revisa `~/.hermes/.env`
- **Error 429 / quota exceeded** → Has fet massa peticions (Gemini gratis). Espera 1 minut o canvia a DeepSeek
- **No em puc connectar per SSH** → Comprova que has posat bé la IP. Algunes VPS triguen 2-3 minuts a estar disponibles

---

> **I2SB · Institut Indústria Sostenible de Barcelona** · Curs d'Instal·lació d'Agents d'IA · Estiu 2026

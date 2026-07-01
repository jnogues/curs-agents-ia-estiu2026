# 💻 Guia 01: Installa i usa OpenCode per programar ESP32

**Durada:** ~30 minuts
**Dificultat:** ⭐⭐ (cal saber una mica de terminal)
**Cost:** 0€ (programari lliure) + API key (~1€/mes)

## Objectiu

Instal·lar OpenCode al teu ordinador i usar-lo per programar ESP32 amb assistència d'IA. Després compilar i pujar el codi amb Arduino IDE. 🚀

---

## Què necessites?

- **Un ordinador** (Windows, macOS o Linux)
- **Cable USB** per connectar l'ESP32
- **Arduino IDE** instal·lat (amb suport per ESP32)
- **Una API key** — la de DeepSeek que ja teniu del curs
- Saber copiar i enganxar comandes al terminal

---

## Pas 1: Instal·la OpenCode

### 🪟 Windows (la majoria d'alumnes)

Obre **PowerShell (o cmd) com a administrador** — botó dret → "Executa com a administrador":

```powershell
npm install -g opencode-ai@latest
```

> 💡 **No tens npm?** Instal·la Node.js (versió 18+) des de [nodejs.org](https://nodejs.org) i torna a executar la comanda.
>
> 🔧 **Error de permisos?** Assegura't d'obrir el terminal **com a administrador**. A Windows, `npm -g` necessita permisos d'admin.

(Tarda 1-2 minuts)

Verifica:

```powershell
opencode --version
```

### 🐧 Linux / macOS

Obre un terminal i executa:

```bash
npm install -g opencode-ai@latest
```

> 💡 **No tens npm?** Instal·la Node.js des de [nodejs.org](https://nodejs.org) o amb el teu gestor de paquets (`sudo apt install nodejs npm` a Ubuntu/Debian).

Verifica:

```bash
opencode --version
```

---

## Pas 2: Configura l'API key de DeepSeek

Agafa la vostra **API key de DeepSeek** (la del curs). Configureu-la al terminal (segons el vostre sistema):

- **PowerShell**: `$env:DEEPSEEK_API_KEY="sk-EL_TEU_TOKEN"`
- **cmd**: `set DEEPSEEK_API_KEY=sk-EL_TEU_TOKEN`
- **Linux/macOS**: `export DEEPSEEK_API_KEY=sk-EL_TEU_TOKEN`

Quan utilitzis OpenCode, especifica el model:

```bash
opencode run "..." --model deepseek/deepseek-chat
```

> 💡 **Consell:** Per no haver d'escriure `--model` cada cop, pots crear un alias:
>
> **PowerShell** (al teu `$PROFILE`):
> ```powershell
> function opencode { & 'opencode' '--model', 'deepseek/deepseek-chat' @args }
> ```
>
> **Linux/macOS** (al `~/.bashrc` o `~/.zshrc`):
> ```bash
> alias opencode='opencode --model deepseek/deepseek-chat'
> ```

> **🔒 Permanent:** Afegeix la variable d'entorn al teu perfil per no haver-la de posar cada cop (com es fa amb Hermes).

---

## Pas 3: Prova que funciona

```bash
opencode run "Explica'm què és un ESP32 en 3 frases"
```

Si tot funciona, veuràs la resposta! 🎉

---

## Pas 4: Programa l'ESP32 amb OpenCode

Ara ve la part divertida! OpenCode pot generar codi per a ESP32.

### Exemple 1: Blink (el clàssic)

```bash
opencode run "Genera un sketch d'Arduino per a ESP32 que faci parpellejar un LED al GPIO2 cada segon. Desa'l a blink.ino"
```

### Exemple 2: Llegir un sensor

```bash
opencode run "Genera un sketch per ESP32 que llegeixi un sensor DHT11 al GPIO4 i mostri temperatura i humitat pel port serie. Desa'l a dht11.ino"
```

### Exemple 3: Connectar-se a WiFi

```bash
opencode run "Genera un sketch per ESP32 que es connecti a WiFi (SSID: 'ElMeuWifi', password: '12345678') i faci una petició HTTP GET a example.com. Desa'l a wifi-test.ino"
```

Cada comanda generarà un fitxer `.ino` que podràs obrir amb l'Arduino IDE.

---

## Pas 5: Revisa i millora codi existent

Al repositori del curs tens exemples a la carpeta `30-code/`. Pots usar OpenCode per revisar-los o millorar-los:

```bash
# Revisar un exercici
opencode run "Revisa aquest codi ESP32 i suggereix millores" -f 30-code/Exercici-01/Exercici-01.ino

# Afegir una funcionalitat
opencode run "Afegeix un mode de baix consum (deep sleep) a aquest codi" -f 30-code/Exercici-01/Exercici-01.ino
```

---

## Pas 6: Compila i puja amb Arduino IDE

Un cop tens el codi generat per OpenCode:

1. **Obre l'Arduino IDE**
2. **File** → **Open** → selecciona el fitxer `.ino` que has creat
3. **Tools** → **Board** → **ESP32 Arduino** → tria el teu model d'ESP32
4. **Tools** → **Port** → selecciona el port USB on tens connectat l'ESP32
5. Clica el botó **→ (Upload)**

Si tot va bé, veuràs el codi compilant-se i pujant-se a l'ESP32! 🎉

> **No tens Arduino IDE amb ESP32?** Segueix aquesta guia ràpida:
> 1. Descarrega Arduino IDE des de [arduino.cc](https://www.arduino.cc/en/software)
> 2. Obre Arduino IDE → **File** → **Preferences**
> 3. A *Additional Boards Manager URLs* afegeix: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
> 4. **Tools** → **Board** → **Boards Manager** → busca "ESP32" i instal·la

---

## Exemple complet pas a pas

Ves al directori del repositori del curs i prova:

```bash
# 1. Ves als exercicis
cd 30-code/

# 2. Demana a OpenCode que analitzi l'Exercici 1
opencode run "Explica'm que fa aquest codi pas a pas" -f Exercici-01/Exercici-01.ino

# 3. Demana una millora
opencode run "Afegeix un segon LED al GPIO5 que s'encengui quan el primer s'apaga" -f Exercici-01/Exercici-01.ino

# 4. Desa la versió millorada
opencode run "Desa la versió millorada a Exercici-01/Exercici-01-millorat.ino"
```

Obre l'Arduino IDE, carrega el fitxer millorat, selecciona la placa i el port, i puja'l! 💡

---

## 🎯 Llista de verificació

- [ ] Node.js instal·lat (`node --version`)
- [ ] OpenCode instal·lat (`opencode --version`)
- [ ] API key configurada (`export OPENROUTER_API_KEY=...` + `--model deepseek/...`)
- [ ] OpenCode respon (`opencode run "Hola"`)
- [ ] Codi ESP32 generat (`.ino`)
- [ ] Arduino IDE amb suport ESP32
- [ ] ESP32 connectat per USB
- [ ] Codi compilat i pujat correctament ✅

---

## 💡 Diferències amb els altres agents del curs

| Aspecte | Hermes | OpenClaw | OpenCode |
|---------|--------|----------|----------|
| Propòsit | Agent general | Agent general | Programació |
| Instal·lació | `curl ... install.sh` | `npm i -g openclaw` | `npm i -g opencode-ai` |
| Interfície | Terminal + TUI | TUI + Gateway | TUI + CLI |
| Ideal per... | Xatejar, cercar, gestionar | Automatitzar canals | Escriure i revisar codi |
| Programar ESP32? | Pot ajudar (xat) | Pot ajudar (xat) | ✅ **Nadiu** |

---

## ❌ Problemes comuns

| Problema | Solució |
|----------|---------|
| **`opencode: command not found`** | Prova `export PATH="$HOME/.npm-global/bin:$PATH"` |
| **`npm install` falla** | Prova amb `sudo npm install -g opencode-ai@latest` (Linux/Mac) |
| **Error d'API key** | Assegura't que has fet `export` de la clau correcta |
| **No troba el port USB** | A l'Arduino IDE: **Tools** → **Port**, prova tots els ports |
| **Error de compilació** | Assegura't que tens la placa ESP32 seleccionada correctament |
| **Error a l'upload** | Mantén premut el botó BOOT de l'ESP32 mentre puja |
| **OpenCode no entén d'ESP32** | Sigues explícit: "per a ESP32, amb Arduino framework" |
| **Error de permisos npm a Windows** | Obre PowerShell o cmd **com a administrador** i prova `npm install -g opencode-ai@latest` de nou |


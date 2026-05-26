# 🖥️ Activitat 00a: Instal·lació local d'Hermes

**Durada:** ~5-10 minuts
**Dificultat:** ⭐ (molt fàcil)
**Cost:** 0€ (només pagaràs l'API Key més endavant)

## Objectiu

Instal·lar Hermes Agent al teu **ordinador personal** per començar a practicar.

## Què necessites?

- Un ordinador amb Windows, macOS o Linux
- Connexió a internet
- Saber obrir un terminal (no cal experiència prèvia)

---

## 👣 Guia pas a pas

### Pas 1: Obre un terminal

| Sistema | Com obrir el terminal |
|---------|-----------------------|
| **Windows** | Clica **Inici** → escriu `cmd` → **Símbol del sistema** (o millor: instal·la [Windows Terminal](https://apps.microsoft.com/detail/9n0dx20hk701) des de la botiga) |
| **macOS** | Cerca **Terminal** al Launchpad o a Aplicacions → Utilitats |
| **Linux (Ubuntu/Debian)** | Cerca **Terminal** al menú d'aplicacions o prem `Ctrl` + `Alt` + `T` |

### Pas 2: Instal·la Python (si no en tens)

Hermes necessita Python 3.10 o superior.

```bash
# Comprova si ja tens Python
python3 --version
```

Si no tens Python o la versió és massa antiga:

| Sistema | Com instal·lar Python |
|---------|----------------------|
| **Windows** | Descarrega'l de [python.org](https://www.python.org/downloads/) i executa l'instal·lador. **Marca l'opció "Add Python to PATH"** |
| **macOS** | `brew install python` (si tens [Homebrew](https://brew.sh)) o des de [python.org](https://www.python.org/downloads/) |
| **Linux** | `sudo apt update && sudo apt install python3 python3-pip python3-venv -y` |

### Pas 3: Instal·la Hermes

Obre un terminal i executa:

```bash
pip install hermes-agent
```

> Si a Windows et dona error, prova `pip3 install hermes-agent` o `python -m pip install hermes-agent`.

> **Alternativa:** Si prefereixes l'instal·lador automàtic (recomanat a Linux/macOS):
> ```bash
> curl -fsSL https://raw.githubusercontent.com/NousResearch/hermes-agent/main/scripts/install.sh | bash
> ```

### Pas 4: Comprova que funciona

```bash
hermes --version
```

Si veus un número de versió (ex: `0.14.0`), **ja ho tens!** 🎉

> **Problema?** Si et diu `hermes: command not found`, prova:
> - **Linux/macOS:** `source ~/.bashrc` o `export PATH="$HOME/.local/bin:$PATH"`
> - **Windows:** Tanca i reobre el terminal, o escriu `python -m hermes --version`

### Pas 5: Configura una API Key

Hermes necessita un model d'IA per funcionar. L'opció més recomanada per començar:

#### 🏆 Recomanat: DeepSeek (~1€/mes)

1. Ves a [platform.deepseek.com](https://platform.deepseek.com) i crea un compte
2. Ves a **API Keys** → **Create API Key**
3. Copia la clau (comença per `sk-...`)

Al terminal:

```bash
# Crea la carpeta de configuració d'Hermes
mkdir -p ~/.hermes

# Desa la teva API Key
cat > ~/.hermes/.env << 'EOF'
DEEPSEEK_API_KEY=sk-EL_TEU_TOKEN_AQUI
EOF

# Configura el model
hermes config set model.provider deepseek
hermes config set model.default deepseek/deepseek-chat
```

> *(Substitueix `sk-EL_TEU_TOKEN_AQUI` per la teva clau real)*

#### Alternativa: Gemini (Gratis — límit de 60 req/minut)

1. Ves a [aistudio.google.com/apikey](https://aistudio.google.com/apikey)
2. Clica **Create API Key**
3. Copia la clau (comença per `AIza...`)

```bash
cat > ~/.hermes/.env << 'EOF'
GEMINI_API_KEY=AIzaEL_TEU_TOKEN_AQUI
EOF

hermes config set model.provider gemini
hermes config set model.default gemini-2.0-flash
```

### Pas 6: Prova'l!

```bash
hermes chat -q "Hola! Soc el teu nou propietari. Com estàs?"
```

Si tot funciona, veuràs la resposta d'Hermes! 🎉

---

## 🎯 Llista de verificació

- [ ] Python 3.10+ instal·lat (`python3 --version`)
- [ ] Hermes instal·lat (`hermes --version`)
- [ ] API Key configurada a `~/.hermes/.env`
- [ ] Model configurat (`hermes config`)
- [ ] Hermes respon (`hermes chat -q "Hola"`)

## ➡️ Següent pas

Ara que tens Hermes funcionant, ves a l'**[Activitat 01: Personalitza el teu agent](../01-personalitza-el-teu-agent/README.md)** per posar-li nom i personalitat!

---

> **I2SB · Institut Indústria Sostenible de Barcelona** · Curs d'Instal·lació d'Agents d'IA · Estiu 2026

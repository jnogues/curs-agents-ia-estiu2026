# 🧿 Hermes Agent — Cheat Sheet Bàsic

> Comandaments essencials per a novells.  
> TUI (terminal), Telegram i Discord.

---

## 📍 Primer contacte

```bash
hermes                  # Xat interactiu
hermes chat -q "consulta"  # Consulta única, surt al moment
hermes setup            # Assistente de configuració inicial
hermes model            # Canviar model / proveïdor
hermes doctor           # Comprovar que tot funciona
hermes --help           # Tots els comandaments
```

---

## 🖥️ TUI / CLI (Terminal)

### Arrencada

| Comandament | Què fa |
|---|---|
| `hermes` | **CLI clàssic** (xat interactiu, sense panells) |
| `hermes --tui` | **TUI gràfic** (interfície amb panells: chat + debug + historial) |
| `hermes -s skill_nom` | Carrega una skill en iniciar |
| `hermes --continue` | Reprèn la darrera sessió |
| `hermes -p perfil` | Usa un perfil concret |

> ⚠️ Per defecte `hermes` obre el **CLI**. Usa `hermes --tui` per a la interfície amb panells.
> Per fer-ho permanent: afegeix `display.interface: tui` al `~/.hermes/config.yaml` o posa `HERMES_TUI=1` al teu shell profile.

### Slash commands (dins del xat)

| Comandament | Què fa |
|---|---|
| `/new` o `/reset` | **Sessió nova** — esborra tot l'historial (màxim estalvi de tokens) |
| `/clear` | Sessió nova + neteja pantalla |
| `/undo` | Desfà l'últim intercanvi |
| `/retry` | Reenvia el teu últim missatge |
| `/compress` | Comprimeix la conversa en un resum (estalvi de tokens) |
| `/title nom` | Posa nom a la sessió |
| `/model nom` | Canvia de model sobre la marxa |
| `/help` | Mostra tots els comandaments |
| `/quit` o `/exit` | Surt del xat |

### Gestió

| Comandament | Què fa |
|---|---|
| `hermes config` | Veure configuració |
| `hermes config set clau valor` | Canvia un valor |
| `hermes config edit` | Obre config.yaml en editor |
| `hermes sessions list` | Llista sessions recents |
| `hermes sessions rename ID nom` | Renombra una sessió |
| `hermes update` | Actualitza Hermes |

---

## 💬 Telegram · Discord (Gateway)

Els mateixos comandaments funcionen a Telegram i Discord.  
Escriu-los al xat amb la barra `/`.

### Essencials

| Comandament | Què fa |
|---|---|
| `/new` o `/reset` | **Sessió nova** — esborra tot l'historial |
| `/undo` | Desfà l'últim intercanvi |
| `/retry` | Reenvia el teu últim missatge |
| `/compress` | Comprimeix la conversa (estalvia tokens) |
| `/title nom` | Posa nom a la sessió |
| `/help` | Mostra ajuda ràpida |
| `/commands` | Navega per tots els comandaments |
| `/status` | Info de la sessió actual |
| `/profile` | Veure el perfil actiu |
| `/usage` | Tokens consumits |
| `/model nom` | Canvia de model |

### Gateway

| Comandament | Què fa |
|---|---|
| `/platforms` o `/gateway` | Veure estat de les plataformes connectades |
| `/sethome` | Marca aquest xat com a **canal principal** |
| `/approve` | **Aprova** un comandament pendent (p. ex. `rm -rf`) |
| `/deny` | **Denega** un comandament pendent |
| `/restart` | Reinicia el gateway |
| `/update` | Actualitza Hermes a la darrera versió |

---

## 🧠 Skills (habilitats)

```bash
hermes skills list          # Llista les skills instal·lades
hermes skills search tema   # Cerca skills al hub
hermes skills install ID    # Instal·la una skill
hermes skills browse        # Explora skills disponibles
/skill nom                  # Carrega una skill dins la sessió
```

Les skills són **receptes reutilitzables** que Hermes pot carregar per fer tasques concretes (depurar, dissenyar, desplegar...).

---

## 🛠️ Eines (Toolsets)

```bash
hermes tools                # Menú interactiu (activa/desactiva eines)
hermes tools list           # Llista totes les eines disponibles
hermes tools enable web     # Activa un toolset (web, terminal, file...)
hermes tools disable web    # Desactiva un toolset
```

Per defecte: **web**, **terminal**, **file**, **code_execution**, **vision**, **memory**, **skills**, **session_search**...

> 💡 Recorda: després de canviar eines, fes `/new` o `/reset` per aplicar-ho.

---

## 🧩 Trucs per a novells

| Situació | Què fer |
|---|---|
| "Hermes no respon" | Comprova `hermes doctor` o `hermes gateway status` |
| "No trobo una sessió" | `hermes sessions list` o `hermes --continue` |
| "Vull canviar de model" | `/model` a mig xat o `hermes model` al terminal |
| "Massiva tokens gastats" | `/new` (esborra tot) o `/compress` (resumeix) |
| "Vull provar un provider" | `hermes setup model` o directament `hermes config set model.provider openrouter` |
| "No em deixa fer alguna cosa" | Respon amb `/approve` al xat |

---

## 📚 Més informació

- **Documentació oficial:** https://hermes-agent.nousresearch.com/docs
- **Skills catalog:** `hermes skills browse`
- **Comandaments complets:** `/help` a qualsevol xat

---

> 🧿 *Hermes: l'agent autònom de codi obert de Nous Research.*

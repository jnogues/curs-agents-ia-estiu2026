# 🦞 Activitat 02: Primera conversa amb el TUI

**Durada:** ~10 minuts
**Dificultat:** ⭐ (fàcil)
**Requisit:** Haver completat la [Guia 01](../01-installa-openclaw-vps/README.md)

## Objectiu

Obrir la interfície gràfica d'OpenClaw (TUI) i descobrir què sap fer! 🎨

---

## Pas 1: Engega el gateway

Abans d'obrir el TUI, el gateway ha d'estar corrent:

```bash
openclaw gateway start
```

Hauries de veure un missatge dient que el gateway s'ha iniciat.

> **Ja el tens corrent?** Si el gateway ja estava actiu, no passa res per tornar-lo a engegar.

---

## Pas 2: Obre el TUI

```bash
openclaw tui
```

El TUI (Terminal User Interface) és una pantalla interactiva amb:
- **Un camp de text** a baix per escriure els teus missatges
- **Les respostes** que van apareixent a la part superior
- **Informació d'estat** (model, connexió, etc.)

> **Nota:** Si el TUI no s'obre, prova primer `openclaw gateway start` i torna-ho a intentar.

---

## Pas 3: Parla amb OpenClaw

Escriu aquests missatges i prem **Enter**:

> *"Hola! Com et dius i què saps fer?"*

> *"Explica'm què és un agent d'IA amb paraules sencilles"*

> *"Fes-me un poema de 4 versos sobre la programació"*

Cada resposta apareixerà a la pantalla. Mira com el TUI mostra les respostes amb format i estil!

---

## Pas 4: Explora el TUI

Prova de fer clic o navegar pels menús del TUI (si en té):

- **Canvia de conversa** — Normalment el TUI permet tenir múltiples xats
- **Configuració ràpida** — Algunes versions permeten canviar de model
- **Historial** — Mira les converses anteriors

> **Consell:** Juga amb la interfície! No la pots trencar 😊

---

## Pas 5: Pregunta coses útils

Ara que ja hi has parlat una mica, prova preguntes pràctiques:

> *"Quin temps fa avui a Barcelona?"*

> *"Explica'm la diferència entre HTML i CSS"*

> *"Tinc un error a Python: 'list index out of range'. Què significa?"*

---

## Per sortir del TUI

Prem **`Ctrl+C`** o **`Esc`** i selecciona **Quit** (o tanca la finestra).

---

## ✨ Què has après?

- Obrir i usar el TUI d'OpenClaw
- Que el TUI és una interfície bonica dins del terminal
- Que OpenClaw pot mantenir converses llargues amb context
- A fer preguntes pràctiques i creatives

## 💡 Per anar més lluny

- Prova de **canviar d'idioma**: demana-li que et respongui en anglès o castellà
- Demana-li que **expliqui un concepte tècnic** com si tinguessis 10 anys
- Obre **dos TUIs alhora** (en finestres de terminal diferents) i compara les respostes

## ❌ No funciona?

| Problema | Solució |
|----------|---------|
| **`command not found`** | Prova `export PATH="$HOME/.npm-global/bin:$PATH"` |
| **El TUI no s'obre** | El gateway ha d'estar corrent: `openclaw gateway start` |
| **No hi ha gateway** | Has completat la Guia 01? `openclaw --version` |
| **No respon** | L'API Key potser no és correcta. Revisa la Guia 01 |
| **Tot falla** | Aviseu el profe! 😅 |

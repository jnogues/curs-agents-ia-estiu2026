# 🧠 Activitat 04: Memòria persistent

**Durada:** ~10 minuts

## Objectiu

Descobrir que Hermes et recorda entre sessions! El que li expliquis avui, ho recordarà demà.

## Passos

### 1. Connecta't i obre Hermes

```bash
ssh userXX@178.105.91.211
export PATH="$HOME/.local/bin:$PATH"
hermes
```

### 2. Explica-li coses sobre tu

> *"Recorda que m'agrada el cafè amb llet i sóc estudiant d'informàtica"*

> *"El meu color preferit és el blau"*

### 3. Tanca la sessió i torna a obrir-la

Surt d'Hermes:

> `/quit`

Torna a obrir:

```bash
hermes
```

### 4. Pregunta-li si se'n recorda

> *"Què saps de mi?"*

> *"De quin color m'agrada?"*

Si ha funcionat, et respondrà correctament! 🎉

## ✨ Què has après?

- Que Hermes té **memòria permanent** (no només dins d'una sessió)
- Que pots desar preferències i dades personals
- Que la memòria persisteix encara que tanquis la sessió
- Que cada alumne té la seva pròpia memòria (independent!)

## 💡 Per anar més lluny

- Demana-li que recordi **coses més complexes** (el teu horari, assignatures favorites...)
- Prova de **contradir-lo**: "Abans t'he dit que m'agradava el cafè, però mentida, prefereixo el te"
- Pregunta-li: *"Quantes coses recordes de mi?"*

## ❌ No funciona?

- Si no recorda res, prova: *"Recorda que..."* en lloc de dir-ho directament
- La memòria no és instantània — de vegades triga un segon a desar-se
- Si no funciona del tot, avisa el profe (potser la memòria està desactivada al VPS)

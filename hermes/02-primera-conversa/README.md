# 💬 Activitat 02: Primera conversa

**Durada:** ~10 minuts

## Objectiu

Xatejar amb Hermes i descobrir què sap fer!

## Passos

### 1. Connecta't i obre Hermes

```bash
ssh userXX@178.105.91.211
export PATH="$HOME/.local/bin:$PATH"
hermes
```

*(Si ja tens la sessió oberta de l'activitat anterior, pots continuar on la vas deixar!)*

### 2. Parla amb ell

Prova de preguntar-li coses variades:

> *"Què saps fer?"*

> *"Escriu-me un poema de 4 versos sobre la intel·ligència artificial"*

> *"Explica'm què és un agent d'IA en 3 frases"*

> *"Fes-me una llista de 5 idees per a un projecte amb Arduino"*

### 3. Prova les comandes /help

Escriu `/help` per veure totes les comandes disponibles.

Prova'n algunes:
- `/model` — quin model d'IA està usant
- `/usage` — quants tokens has gastat
- `/status` — informació de la sessió

### 4. Canvia de tema sense perdre el fil

Hermes recorda la conversa anterior. Prova:

> *"Recordes el poema que m'has escrit abans? Millora'l!"*

## ✨ Què has après?

- Que Hermes entén català, castellà, anglès...
- Que pot ser creatiu (poemes, idees, històries)
- Que té memòria dins de la mateixa sessió
- Les comandes `/help` per explorar

## 💡 Per anar més lluny

- Demana-li que **expliqui un concepte tècnic** com si tinguessis 10 anys
- Demana-li que **comparin dues eines** (ex: "Python vs JavaScript per a IA")
- Prova `/model` i canvia de model si el profe t'ho autoritza

## ❌ No funciona?

- Si Hermes no respon, espera uns segons i torna-ho a intentar
- Si veus un error `401` o `429`, avisa el profe (potser la clau API ha arribat al límit)

# 🎭 Activitat 01: Personalitza el teu agent

**Durada:** ~5 minuts

## Objectiu

Donar personalitat al teu Hermes! Li posaràs un nom i li diràs com vols que et parli.

## Passos

### 1. Connecta't al VPS

```bash
ssh userXX@178.105.91.211
```
*(Substitueix XX pel teu número d'usuari: 01, 02, 03, 04 o 05)*
*Contrasenya: **1234***

### 2. Obre Hermes

```bash
export PATH="$HOME/.local/bin:$PATH"
hermes
```

### 3. Personalitza'l

Escriu això a la xat:

> *"A partir d'ara et diràs **Sparky** i em parlaràs en català informal i amb molt d'humor."*

(Pots canviar "Sparky" pel nom que vulguis!)

### 4. Comprova que funciona

Pregunta-li:

> *"Com et dius? Explica'm un acudit!"*

## ✨ Què has après?

- Que Hermes obeeix instruccions al moment
- Que pots canviar com et parla sense tocar fitxers
- Que cada alumne pot tenir un Hermes diferent al seu gust

## 💡 Per anar més lluny

Prova diferents personalitats:
- *"A partir d'ara ets un assistent formal que em parla de vostè"*
- *"Ets un pirata informàtic, parla'm amb argot hacker"*
- *"Ets un mestre iuguià d'sushi, dóna'm consells culinaris"*

## ❌ No funciona?

- Assegura't que tens `hermes` instal·lat: `hermes --version`
- Prova `export PATH="$HOME/.local/bin:$PATH"` abans de `hermes`
- Pregunta al profe!

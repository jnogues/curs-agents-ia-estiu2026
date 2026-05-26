# 🐳 Activitat 00c: Docker + Serveis al VPS

**Durada:** ~20-30 minuts
**Dificultat:** ⭐⭐ (saber copiar i enganxar)
**Cost:** 0€ extra (ja tens el VPS)

## Objectiu

Desplegar un stack complet de serveis al teu VPS usant un sol **prompt one-shot** per a Hermes. Ensenyaràs al teu agent a instal·lar Docker i configurar:

| Servei | Què fa | Port |
|--------|--------|:----:|
| 🐳 **Docker** | Motor de contenidors | — |
| 📦 **Dockge** | Gestor visual de stacks Docker | 5001 |
| 📡 **Mosquitto** | Broker MQTT (IoT) | 1883 |
| ⏱️ **InfluxDB** | Base de dades de sèries temporals | 8086 |
| 📊 **Grafana** | Dashboards i visualització | 3000 |
| 🔧 **Node-RED** | Programació visual low-code | 1880 |

> 💡 **Per què aquests serveis?** Són el stack perfecte per IoT: sensors publiquen a MQTT → InfluxDB guarda les dades → Grafana les visualitza. I Dockge t'ajuda a gestionar-ho tot des d'un navegador.

---

## Prerequisits

- [ ] Has completat la [Instal·lació en VPS](../00b-instalacio-vps/README.md) (tens Hermes al VPS)
- [ ] Tens accés SSH al teu VPS
- [ ] Pots executar `hermes chat` sense errors

---

## Com funciona?

Aquesta activitat utilitza un **prompt one-shot**: és un text que li dones a Hermes i ell fa tota la feina. No has de picar comandes manualment — Hermes s'encarrega de tot, pas per pas, verificant cada servei abans de passar al següent.

### Pas 1: Connecta't al VPS

Des del teu ordinador:

```bash
ssh root@<IP_DEL_VPS>
```

### Pas 2: Obre Hermes

```bash
hermes chat
```

Espera que aparegui el missatge de "Connectat" i que aparegui el cursor `💬 >`.

### Pas 3: Dona-li el prompt

Obre el fitxer [`prompt-one-shot.md`](./prompt-one-shot.md), **copia tot el text** i enganxa'l al xat d'Hermes.

> ⚠️ **Important:** Enganxa tot el text de cop, no per parts. El prompt conté la seqüència completa i Hermes anirà fent cada pas i verificant-lo.

### Pas 4: Deixa que Hermes treballi

Hermes anirà:
1. Instal·lant Docker
2. Configurant cada servei un per un
3. Verificant que cada un respon
4. Mostrant-te una taula resum al final

**Asseu-te i mira com treballa!** 🍿

---

## Què hauries de veure al final

Quan Hermes acabi, et mostrarà una taula com aquesta:

| Servei | Port | URL | Credencials per defecte |
|--------|:----:|-----|------------------------|
| Dockge | 5001 | http://IP:5001 | Sense auth (configurar al primer accés) |
| Mosquitto | 1883 | tcp://IP:1883 | Anònim |
| InfluxDB | 8086 | http://IP:8086 | `admin` / `CANVIAR_AQUESTA_CONTRASENYA` |
| Grafana | 3000 | http://IP:3000 | Admin: `admin` / `admin` |
| Node-RED | 1880 | http://IP:1880 | Sense auth |

> ⚠️ **Recorda canviar les contrasenyes per defecte!** Sobretot les d'InfluxDB i Grafana.

---

## 🖥️ Prova'ls al navegador

Des del teu ordinador, obre al navegador (substitueix `IP` per la del teu VPS):

- **Dockge:** `http://IP:5001` — Gestiona els teus stacks
- **Grafana:** `http://IP:3000` — Visualitza dades (admin/admin)
- **Node-RED:** `http://IP:1880` — Crea fluxos low-code
- **InfluxDB:** `http://IP:8086` — API de base de dades

> **Només funcionen si el firewall ho permet.** Si tens `ufw` activat, obre els ports:
> ```bash
> ufw allow 1883  # Mosquitto
> ufw allow 3000  # Grafana
> ufw allow 5001  # Dockge
> ufw allow 8086  # InfluxDB
> ufw allow 1880  # Node-RED
> ```

---

## 🎯 Llista de verificació

- [ ] Hermes instal·lat al VPS
- [ ] He copiat i enganxat el prompt a Hermes
- [ ] Hermes ha instal·lat Docker
- [ ] Hermes ha configurat tots els serveis
- [ ] Hermes ha verificat que cada servei respon
- [ ] Puc accedir a Dockge des del navegador (port 5001)
- [ ] Puc accedir a Grafana des del navegador (port 3000)
- [ ] He canviat les contrasenyes per defecte

---

## ❌ Problemes comuns

- **`sudo: a password is required`** → El prompt ja ho resol automàticament (configura NOPASSWD)
- **Dockge no obre** → `ufw` pot estar bloquejant el port 5001. Executa `ufw allow 5001`
- **Mosquitto no respon** → `sg docker -c "docker logs mosquitto"` per veure errors
- **El VPS es queda sense disc** → `docker system prune -a` per netejar contenidors aturats
- **El prompt no funciona** → Assegura't que l'has enganxat sencer, no per parts

---

## ➡️ I ara què?

- **Vols programar l'ESP32?** Ves al [Mòdul 30: Exercicis de codi](../../../30-code/README.md)
- **Vols fluxos IoT amb Node-RED?** Prova a connectar Node-RED al teu broker MQTT
- **Vols dashboards xulos?** A Grafana, ves a **Configuration → Data Sources** i connecta InfluxDB

---

> **I2SB · Institut Indústria Sostenible de Barcelona** · Curs d'Instal·lació d'Agents d'IA · Estiu 2026

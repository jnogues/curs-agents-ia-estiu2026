# 🌐 Activitat 00d: Hermes WebUI (Interfície gràfica)

**Durada:** ~15-20 minuts (tu) + ~5 minuts d'Hermes treballant
**Dificultat:** ⭐⭐ (cal obrir duckdns.org al navegador)
**Cost:** 0€ extra (ja tens el VPS)

## Objectiu

Donar-li una **cara bonica al teu Hermes**. Fins ara només el podies fer servir per terminal (SSH), Telegram o Discord. Amb aquesta activitat tindràs una **interfície web** accessible des de qualsevol navegador:

| Abans | Després |
|-------|---------|
| 🖥️ `hermes chat` al terminal | 🌐 `https://hermes.tudomini.duckdns.org` al navegador |
| ⌨️ Només escriure comandes | 🖱️ Interfície gràfica completa |
| 🔒 Accés local o apps | 🔓 Accés des de qualsevol dispositiu amb HTTPS |

| Servei | Què fa | Port |
|--------|--------|:----:|
| 🦆 **DuckDNS** | Manté el teu domini actualitzat | — |
| 🐇 **Caddy** | Servidor web amb HTTPS automàtic | 80/443 |
| 🌐 **Hermes WebUI** | Interfície web per al teu agent | 8787 |

> 💡 **La màgia:** Caddy demana certificats HTTPS a Let's Encrypt automàticament. No cal configurar res de SSL.

---

## Prerequisits

- [ ] Tens **Hermes instal·lat al VPS** ([Activitat 00b](../00b-instalacio-vps/README.md))
- [ ] Tens **Docker instal·lat** ([Activitat 00c](../00c-docker-serveis-vps/README.md) o pel teu compte)
- [ ] Pots connectar-te per SSH al VPS i executar `hermes chat`

---

## Com funciona?

Aquesta activitat té **dos grans blocs**:

### Bloc manual (tu, al navegador): Donar-se d'alta a DuckDNS

DuckDNS és un servei **gratuït** que assigna un domini del tipus `eltudomini.duckdns.org` al teu VPS. Com que les IPs públiques poden canviar, DuckDNS les manté actualitzades automàticament.

**Has de fer 3 coses al navegador:**

1. **Ves a [duckdns.org](https://duckdns.org)**
2. **Inicia sessió** amb GitHub, Google, Twitter o el mètode que prefereixis
3. **Crea un subdomini** (posa el nom que vulguis, ex: `joanmaria`) i **copia el token** que apareix

> ⚠️ **Guarda el token**, el necessitaràs al prompt one-shot!

### Bloc automàtic (Hermes): Instal·lar DuckDNS, Caddy i la WebUI

Obre el fitxer [`prompt-one-shot.md`](./prompt-one-shot.md) i **substitueix els 3 placeholders** que hi ha al principi:

| Placeholder | Què hi poses |
|-------------|--------------|
| `<SUBDOMINI>` | El nom que has posat a DuckDNS (ex: `joanmaria`) |
| `<TOKEN>` | El token que t'ha donat DuckDNS |
| `<PASSWORD>` | Una contrasenya per entrar a la WebUI (la que vulguis) |

Després, connecta't al VPS per SSH, obre `hermes chat`, i **enganxa tot el text**.

---

## 🖥️ Al final tindràs

```
1. DuckDNS container → manté el teu domini viu
2. Caddy container → HTTPS automàtic al teu domini
3. Hermes WebUI → accessible a:
   🌐 https://hermes.<SUBDOMINI>.duckdns.org
```

Obre el navegador, ves a l'URL, i posa la contrasenya que has triat. **Ja tens Hermes al navegador!** 🎉

---

## 🎯 Llista de verificació

- [ ] M'he registrat a duckdns.org
- [ ] He creat un subdomini i tinc el token
- [ ] He substituït els 3 placeholders al prompt
- [ ] He enganxat el prompt a Hermes
- [ ] Hermes ha construït la imatge Docker
- [ ] Hermes ha configurat DuckDNS, Caddy i la WebUI
- [ ] Puc obrir `https://hermes.<SUBDOMINI>.duckdns.org` al navegador
- [ ] Puc iniciar sessió amb la meva contrasenya

---

## ❌ Problemes comuns

- **`duckdns.org` no carrega** → Prova més tard, de vegades va lent
- **El domini no funciona** → DuckDNS pot trigar uns minuts a propagar-se. Pots veure l'estat a `https://duckdns.org?domains=<SUBDOMINI>`
- **HTTPS no funciona** → Caddy necessita que el domini apunti al teu VPS. Comprova a `https://dnschecker.org` que `hermes.<SUBDOMINI>.duckdns.org` resolgui a la IP del teu VPS
- **Port 80 o 443 ocupats** → Si ja tens un altre servidor web, Caddy no podrà lligar els ports. Atura'l primer o consulta al profe
- **Caddy no pot crear el directori** → Hermes ho gestiona, però si falla, prova `mkdir -p ~/caddy` manualment
- **`docker build` triga molt** → La primera construcció baixa la imatge base Python (~130MB). Pots anar a fer un cafè ☕
- **No recordo el token de DuckDNS** → Ves a duckdns.org, inicia sessió, i el veuràs a la pàgina principal

---

## ➡️ I ara què?

- **Vols afegir més serveis al Caddy?** Edita `~/caddy/Caddyfile` i afegeix entrades com `grafana.<SUBDOMINI>.duckdns.org` → reinicia Caddy (`docker compose -f ~/caddy/docker-compose.yml restart`)
- **Vols explorar la WebUI?** Prova de crear un workspace, gestionar cron jobs, o canviar de model des de la interfície
- **Torna a l'índex d'instal·lació** a [00-README](../README.md)

---

> **I2SB · Institut Indústria Sostenible de Barcelona** · Curs d'Instal·lació d'Agents d'IA · Estiu 2026

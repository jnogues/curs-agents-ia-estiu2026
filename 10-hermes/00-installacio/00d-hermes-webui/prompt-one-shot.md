# 🌐 One-shot: Hermes WebUI amb DuckDNS + Caddy + HTTPS

> **Com usar-lo:**
> 1. Ves a duckdns.org, registra't i crea un subdomini. Copia el token.
> 2. **Substitueix** `<SUBDOMINI>`, `<TOKEN>` i `<PASSWORD>` aquí al prompt (són 3 llocs).
> 3. Connecta't per SSH al VPS, obre `hermes chat`, i enganxa **tot el text** de cop.
>
> **Avís:** El primer cop triga uns minuts perquè construeix la imatge Docker des de zero. Deixa'l fer!

---

**Placeholders que has de substituir ABANS d'enganxar:**

- `<SUBDOMINI>` → El nom que has posat a DuckDNS (ex: `joanmaria`)
- `<TOKEN>` → El token de DuckDNS que has copiat
- `<PASSWORD>` → Una contrasenya per al teu Hermes WebUI (la que vulguis)

---

## Fase 0 — Reconeixement i preparació

Abans de començar, comprovo que tot està a punt:

```bash
# Docker instal·lat?
sg docker -c "docker --version" || { echo "❌ Docker no està instal·lat. Fes l'activitat 00c primer!"; exit 1; }

# Ports lliures?
for port in 80 443 8787; do
  if ss -tlnp | grep -q ":$port "; then
    echo "⚠️  Port $port ocupat. El necessito per al WebUI."
  else
    echo "✅ Port $port lliure"
  fi
done

# Usuari
echo "✅ Usuari: $(whoami) (UID:$(id -u))"
echo "✅ Home: $HOME"
```

---

## Fase 1 — DuckDNS (mantenidor de domini)

El container DuckDNS s'encarregarà de mantenir `SUBDOMINI.duckdns.org` apuntant a la IP del teu VPS.

```bash
mkdir -p ~/duckdns
```

Crea `~/duckdns/docker-compose.yml`:

```yaml
services:
  duckdns:
    image: lscr.io/linuxserver/duckdns:latest
    container_name: duckdns
    environment:
      - PUID=1000
      - PGID=1000
      - TZ=Europe/Madrid
      - SUBDOMAINS=<SUBDOMINI>
      - TOKEN=<TOKEN>
    restart: unless-stopped
```

```bash
cd ~/duckdns && sg docker -c "docker compose up -d"
```

Verificació:

```bash
sleep 5
sg docker -c "docker logs duckdns 2>&1 | tail -3"
```

> ✅ DuckDNS ha d'escriure alguna cosa com `OK` o la IP del VPS.

---

## Fase 2 — Caddy (servidor web amb HTTPS automàtic)

Caddy escoltarà als ports 80 i 443 i et donarà HTTPS gratis amb Let's Encrypt.

```bash
mkdir -p ~/caddy/data
```

Crea `~/caddy/Caddyfile`:

```
hermes.<SUBDOMINI>.duckdns.org {
    reverse_proxy 172.17.0.1:8787
}
```

Crea `~/caddy/docker-compose.yml`:

```yaml
services:
  caddy:
    image: caddy:latest
    container_name: caddy
    restart: unless-stopped
    ports:
      - 80:80
      - 443:443
      - 443:443/udp
    volumes:
      - ./Caddyfile:/etc/caddy/Caddyfile
      - ./data:/data
```

```bash
cd ~/caddy && sg docker -c "docker compose up -d"
```

Verificació:

```bash
sleep 5
sg docker -c "docker logs caddy 2>&1 | tail -5"
```

> ⏳ Caddy pot trigar uns segons a demanar el certificat. No passa res si veus warnings al principi.

---

## Fase 3 — Construir la imatge del Hermes WebUI

Clono el repositori de nesquena i construeixo la imatge Docker:

```bash
# Clonar (o actualitzar si ja existeix)
if [ -d ~/hermes-webui-repo ]; then
  cd ~/hermes-webui-repo && git pull
else
  git clone https://github.com/nesquena/hermes-webui.git ~/hermes-webui-repo
fi
```

```bash
cd ~/hermes-webui-repo && sg docker -c "docker build -t hermes-webui ."
```

Verificació:

```bash
sg docker -c "docker images hermes-webui --format '{{.Repository}}:{{.Tag}} {{.Size}}'"
```

> ✅ Ha de mostrar `hermes-webui:latest` amb una mida (~130MB o més).

---

## Fase 4 — Desplegar el Hermes WebUI

```bash
mkdir -p ~/hermes-webui
```

Crea `~/hermes-webui/docker-compose.yml`:

```yaml
services:
  hermes-webui:
    image: hermes-webui:latest
    container_name: hermes-webui
    ports:
      - "8787:8787"
    volumes:
      - /home/$USER/.hermes:/home/hermeswebui/.hermes
      - /home/$USER/workspace:/workspace
    environment:
      - WANTED_UID=$(id -u)
      - WANTED_GID=$(id -g)
      - HERMES_WEBUI_HOST=0.0.0.0
      - HERMES_WEBUI_PORT=8787
      - HERMES_WEBUI_STATE_DIR=/home/hermeswebui/.hermes/webui
      - HERMES_WEBUI_PASSWORD=<PASSWORD>
    restart: unless-stopped
```

```bash
cd ~/hermes-webui && sg docker -c "docker compose up -d"
```

Verificació:

```bash
sleep 5
sg docker -c "docker inspect hermes-webui --format '{{.State.Health.Status}}'"
curl -s -o /dev/null -w "✅ HTTP: %{http_code}\n" --max-time 5 http://localhost:8787/health
```

> ✅ Ha de mostrar `healthy` i HTTP `200`.

---

## Fase 5 — Verificació final

```bash
echo ""
echo "═══════════════════════════════════════"
echo "  ✅ INSTAL·LACIÓ COMPLETADA!"
echo "═══════════════════════════════════════"
echo ""
echo "🌐 Obre al navegador:"
echo "   https://hermes.<SUBDOMINI>.duckdns.org"
echo ""
echo "🔑 Contrasenya: la que has posat al prompt"
echo ""

# Prova HTTPS
curl -s -o /dev/null -w "📡 HTTPS: %{http_code} (hermes.<SUBDOMINI>.duckdns.org)\n" \
  --max-time 10 https://hermes.<SUBDOMINI>.duckdns.org 2>&1 || \
  echo "📡 HTTPS: encara no accessible (pot trigar 1-2 minuts que Caddy obteni el certificat)"
```

Mostra'm la taula final:

| Servei | Container | Port | Accés |
|--------|-----------|:----:|-------|
| 🦆 DuckDNS | duckdns | — | Manté `SUBDOMINI.duckdns.org` actualitzat |
| 🐇 Caddy | caddy | 80 / 443 | Proxy invers + HTTPS automàtic |
| 🌐 Hermes WebUI | hermes-webui | 8787 | `https://hermes.SUBDOMINI.duckdns.org` |

> **Nota:** Si Caddy no ha pogut obtenir el certificat en 2 minuts, prova `sg docker -c "docker logs caddy"` per veure l'estat. Normalment funciona sol als pocs minuts.

---

## ↩️ Rollback (si alguna cosa va malament)

Si vols desfer-ho tot:

```bash
# Aturar i eliminar containers
cd ~/hermes-webui && sg docker -c "docker compose down -v"
cd ~/caddy && sg docker -c "docker compose down -v"
cd ~/duckdns && sg docker -c "docker compose down -v"

# Eliminar directoris
rm -rf ~/hermes-webui ~/caddy ~/duckdns ~/hermes-webui-repo

# Eliminar la imatge Docker
sg docker -c "docker rmi hermes-webui"
```

Això deixa el sistema tal com estava abans de començar.

# 🐳 One-shot: Instal·lar Docker + Stack de Serveis al VPS

> **Versió revisada** — amb verificacions correctes, reversibilitat i seguretat.
>
> **Com usar-lo:** Connecta't per SSH al VPS, obre `hermes chat`, i enganxa tot aquest text de cop.

---

**Abans de començar:** Aquest prompt **captura l'estat previ** per poder desfer-ho tot si cal.
Si alguna cosa falla a mig camí, atura'm i demana'm el **rollback**.

---

## Fase 0 — Captura d'estat previ (per reversibilitat)

```bash
# Guardem l'estat inicial per al rollback
docker ps -a > /tmp/pre_docker_state.txt 2>/dev/null || true
dpkg -l | grep -i docker > /tmp/pre_docker_packages.txt 2>/dev/null || true
echo "Estat previ capturat a /tmp/pre_*"
```

---

## Fase 1 — Instal·lar Docker

```bash
curl -fsSL https://get.docker.com | sudo sh
```

Afegeix l'usuari al grup `docker`:

```bash
sudo usermod -aG docker $USER
```

Configura **NOPASSWD només per Docker** (no total):

```bash
echo "$USER ALL=(ALL) NOPASSWD: /usr/bin/docker, /usr/bin/sg" | sudo tee /etc/sudoers.d/$USER-docker
```

Verifica:

```bash
sg docker -c "docker --version && docker compose version"
```

✅ **Fet.** Si falla aquí, demana'm rollback.

---

## Fase 2 — Dockge (gestor de stacks UI, port 5001)

```bash
mkdir -p ~/dockge
```

Crea `~/dockge/docker-compose.yml`:

```yaml
services:
  dockge:
    image: louislam/dockge:latest
    container_name: dockge
    restart: unless-stopped
    ports:
      - 5001:5001
    volumes:
      - /var/run/docker.sock:/var/run/docker.sock
      - ./data:/app/data
      - /home/$USER:/home/$USER
    environment:
      - DOCKGE_STACKS_DIR=/home/$USER
```

```bash
cd ~/dockge && sg docker -c "docker compose up -d"
```

**Verificació:**

```bash
curl -s -o /dev/null -w "%{http_code}" --max-time 5 http://localhost:5001
```

→ Ha de tornar **200**.

✅ **Fet.**

---

## Fase 3 — Mosquitto (MQTT broker, port 1883, accés anònim)

```bash
mkdir -p ~/mosquitto/{config,data,log}
```

Crea `~/mosquitto/config/mosquitto.conf`:

```
listener 1883
allow_anonymous true
persistence true
persistence_location /mosquitto/data/
log_dest file /mosquitto/log/mosquitto.log
log_dest stdout
connection_messages true
log_type all
```

Crea `~/mosquitto/docker-compose.yml`:

```yaml
services:
  mosquitto:
    image: eclipse-mosquitto:latest
    container_name: mosquitto
    restart: unless-stopped
    ports:
      - 1883:1883
    volumes:
      - ./config:/mosquitto/config
      - ./data:/mosquitto/data
      - ./log:/mosquitto/log
```

Permisos (Mosquitto corre com UID 1883):

```bash
sudo chown -R 1883:1883 ~/mosquitto/data ~/mosquitto/log
```

```bash
cd ~/mosquitto && sg docker -c "docker compose up -d"
```

**Verificació** — cal instal·lar `mosquitto-clients` a l'host (el contenidor NO inclou `mosquitto_sub`):

```bash
sudo apt install -y mosquitto-clients
sg docker -c "mosquitto_sub -h localhost -t '\$SYS/#' -v -W 3"
```

→ Ha de mostrar info del broker (versió, uptime, etc.). Si tot va bé, surt automàticament als 3 segons. Si no veus res, revisa que el contenidor estigui en marxa.

✅ **Fet.**

---

## Fase 4 — InfluxDB 1.11 (time-series DB, port 8086)

```bash
mkdir -p ~/influxdb/{data,grafana-provisioning/datasources}
```

Crea `~/influxdb/docker-compose.yml`:

```yaml
services:
  influxdb:
    image: influxdb:1.11
    container_name: influxdb
    restart: unless-stopped
    ports:
      - 8086:8086
    volumes:
      - ./data:/var/lib/influxdb
    environment:
      - INFLUXDB_DB=sensors
      - INFLUXDB_ADMIN_USER=admin
      - INFLUXDB_ADMIN_PASSWORD=canviar_aquesta_contrasenya
      - INFLUXDB_HTTP_AUTH_ENABLED=true
```

Crea `~/influxdb/grafana-provisioning/datasources/influxdb.yaml`:

```yaml
apiVersion: 1
datasources:
  - name: InfluxDB
    type: influxdb
    access: proxy
    url: http://influxdb:8086
    database: sensors
    user: admin
    secureJsonData:
      password: canviar_aquesta_contrasenya
    isDefault: true
```

Permisos (InfluxDB corre com UID 1500):

```bash
sudo chown -R 1500:1500 ~/influxdb/data
```

```bash
cd ~/influxdb && sg docker -c "docker compose up -d"
```

**Verificació:**

```bash
curl -u admin:canviar_aquesta_contrasenya http://localhost:8086/query?q=SHOW+DATABASES
```

→ Ha de tornar un JSON amb la base de dades `sensors`.

✅ **Fet.**

---

## Fase 5 — Grafana (visualització, port 3000)

Afegeix al mateix `docker-compose.yml` d'InfluxDB (edita `~/influxdb/docker-compose.yml` i afegeix sota influxdb):

```yaml
  grafana:
    image: grafana/grafana:latest
    container_name: grafana
    restart: unless-stopped
    ports:
      - 3000:3000
    volumes:
      - ./grafana-data:/var/lib/grafana
      - ./grafana-provisioning:/etc/grafana/provisioning
    environment:
      - GF_AUTH_ANONYMOUS_ENABLED=true
      - GF_AUTH_ANONYMOUS_ORG_ROLE=Viewer
      - GF_SECURITY_ADMIN_USER=admin
      - GF_SECURITY_ADMIN_PASSWORD=admin
```

Ara sí, permisos en l'ordre correcte (primer `mkdir`, després `chown`):

```bash
mkdir -p ~/influxdb/grafana-data
sudo chown -R 472:472 ~/influxdb/grafana-data
```

```bash
cd ~/influxdb && sg docker -c "docker compose up -d"
```

**Verificació:**

```bash
curl -s -o /dev/null -w "%{http_code}" --max-time 5 http://localhost:3000
```

→ Ha de tornar **200** o **302** (redirecció al login).

✅ **Fet.**

---

## Fase 6 — Node-RED (low-code, port 1880)

```bash
mkdir -p ~/nodered/data
```

Crea `~/nodered/docker-compose.yml`:

```yaml
services:
  nodered:
    image: nodered/node-red:latest
    container_name: nodered
    restart: unless-stopped
    ports:
      - 1880:1880
    environment:
      - TZ=Europe/Madrid
    volumes:
      - ./data:/data
    user: "1000:1000"
```

```bash
cd ~/nodered && sg docker -c "docker compose up -d"
```

**Verificació:**

```bash
curl -s -o /dev/null -w "%{http_code}" --max-time 5 http://localhost:1880
```

→ Ha de tornar **200**.

✅ **Fet.**

---

## Resum de serveis

| Servei    | Port | URL                     | Credencials per defecte                           |
|-----------|:----:|-------------------------|---------------------------------------------------|
| Dockge    | 5001 | http://IP:5001          | Sense auth (configurar al primer accés)           |
| Mosquitto | 1883 | tcp://IP:1883           | Anònim                                            |
| InfluxDB  | 8086 | http://IP:8086          | `admin` / `canviar_aquesta_contrasenya`           |
| Grafana   | 3000 | http://IP:3000          | Admin: `admin` / `admin` — Visites: anònim Viewer |
| Node-RED  | 1880 | http://IP:1880          | Sense auth                                        |

> ⚠️ **Recorda canviar les contrasenyes per defecte!** Especialment la d'InfluxDB.

---

## 🪝 Rollback (desinstal·lar-ho tot)

Si necessites desfer tota la instal·lació, executa això:

```bash
# 1. Aturar i eliminar tots els contenidors
cd ~/dockge && sg docker -c "docker compose down -v" 2>/dev/null || true
cd ~/mosquitto && sg docker -c "docker compose down -v" 2>/dev/null || true
cd ~/influxdb && sg docker -c "docker compose down -v" 2>/dev/null || true
cd ~/nodered && sg docker -c "docker compose down -v" 2>/dev/null || true

# 2. Eliminar directoris dels stacks
rm -rf ~/dockge ~/mosquitto ~/influxdb ~/nodered

# 3. Desinstal·lar Docker
sudo apt purge -y docker-ce docker-ce-cli docker-ce-rootless-extras docker-buildx-plugin docker-compose-plugin 2>/dev/null || true
sudo apt autoremove -y 2>/dev/null || true

# 4. Neteja de grups i sudo
sudo deluser $USER docker 2>/dev/null || true
sudo rm -f /etc/sudoers.d/$USER-docker

# 5. Opcional: netejar tot el sistema Docker (imatges, volums, etc.)
# sudo rm -rf /var/lib/docker

echo "✅ Rollback completat. El sistema ha tornat a l'estat previ (aproximadament)."
```

> **Nota:** El rollback no restaura paquets instal·lats per verificacions (`mosquitto-clients`). Si vols neteja absoluta, afegeix `sudo apt purge -y mosquitto-clients` al rollback.
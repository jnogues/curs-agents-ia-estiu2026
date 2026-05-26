# 🐳 Prompt one-shot: Instal·lar Docker + Serveis al VPS

> **Com usar-lo:** Connecta't per SSH al teu VPS, obre `hermes chat`, i enganxa tot aquest text de cop. Hermes farà la resta.

---

Hola Hermes! Necessito que instal·lis Docker i un stack de serveis al meu VPS (Ubuntu acabat d'instal·lar). Vull una instal·lació neta amb els directoris dels stacks a `~/` perquè Dockge els pugui gestionar.

**Fes-ho en aquest ordre, un per un, verificant que cada pas funciona abans de passar al següent:**

---

## 1. Instal·lar Docker

```bash
curl -fsSL https://get.docker.com | sudo sh
```

Després, afegeix el meu usuari al grup docker:
```bash
sudo usermod -aG docker $USER
```

Verifica que Docker funciona (primera comanda amb `sg docker -c`, després ja normal):
```bash
sg docker -c "docker --version && docker compose version"
```

**Important:** Si sudo requereix contrasenya, configura NOPASSWD primer:
```bash
echo "$USER ALL=(ALL) NOPASSWD: ALL" | sudo tee /etc/sudoers.d/$USER-nopasswd
```

---

## 2. Dockge (gestor de stacks UI, port 5001)

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

Verifica que respon: `curl -s -o /dev/null -w "%{http_code}" --max-time 5 http://localhost:5001` → ha de tornar 200.

---

## 3. Mosquitto (MQTT broker, port 1883, accés anònim)

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

Verifica: `sg docker -c "docker exec mosquitto mosquitto_sub -t '\$SYS/#' -v -W 3"` → Ha de mostrar info del broker.

---

## 4. InfluxDB 1.11 (time-series DB, port 8086)

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
      - INFLUXDB_ADMIN_PASSWORD=CANVIAR_AQUESTA_CONTRASENYA
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
      password: CANVIAR_AQUESTA_CONTRASENYA
    isDefault: true
```

Permisos (InfluxDB corre com UID 1500):
```bash
sudo chown -R 1500:1500 ~/influxdb/data
```

```bash
cd ~/influxdb && sg docker -c "docker compose up -d"
```

Verifica: `curl -u admin:CANVIAR_AQUESTA_CONTRASENYA http://localhost:8086/query?q=SHOW+DATABASES` → Ha de mostrar `sensors`.

---

## 5. Grafana (visualització, port 3000)

**Afegeix al mateix docker-compose d'InfluxDB** (edita `~/influxdb/docker-compose.yml` i afegeix sota el servei influxdb):

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

Permisos (Grafana corre com UID 472):
```bash
sudo chown -R 472:472 ~/influxdb/grafana-data
mkdir -p ~/influxdb/grafana-data
```

```bash
cd ~/influxdb && sg docker -c "docker compose up -d"
```

Verifica: `curl -s -o /dev/null -w "%{http_code}" --max-time 5 http://localhost:3000` → 200 o 302.

---

## 6. Node-RED (low-code, port 1880)

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

Verifica: `curl -s -o /dev/null -w "%{http_code}" --max-time 5 http://localhost:1880` → 200.

---

## Quan acabis, mostra'm una taula com aquesta:

| Servei | Port | URL | Credencials per defecte |
|--------|:----:|-----|------------------------|
| Dockge | 5001 | http://IP:5001 | Sense auth (configurar al primer accés) |
| Mosquitto | 1883 | tcp://IP:1883 | Anònim |
| InfluxDB | 8086 | http://IP:8086 | `admin` / `CANVIAR_AQUESTA_CONTRASENYA` |
| Grafana | 3000 | http://IP:3000 | Admin: `admin` / `admin` — Visites: anònim Viewer |
| Node-RED | 1880 | http://IP:1880 | Sense auth |

**Recorda'm després que canviï les contrasenyes per defecte!**

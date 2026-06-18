# Instal·lar Node.js (prompt per a OpenCode)

Copia i enganxa això a OpenCode:

```
Vull instal·lar Node.js al meu ordinador per poder usar OpenCode (necessito npm).

Primer detecta si el meu sistema és Windows o Ubuntu/Linux.

A Ubuntu/Linux:

1. Comprova si ja el tinc instal·lat:
   node --version

2. Si no el tinc, instal·la'l des del repositori oficial:
   curl -fsSL https://deb.nodesource.com/setup_22.x | sudo -E bash -
   sudo apt install -y nodejs

3. Verifica:
   node --version   (ha de ser 18+)
   npm --version

A Windows:

1. Ves a https://nodejs.org/ i descarrega la versió LTS (la recomanada)

2. Executa l'instal·lador:
   - Deixa tot per defecte (Next, Next...)
   - Assegura't que marca "Add to PATH"

3. Un cop instal·lat, obre un terminal nou i verifica:
   node --version
   npm --version

Un cop instal·lat (ambdós sistemes):

Ara ja pots instal·lar OpenCode:
npm install -g opencode-ai@latest

Si algun pas falla, explica'm l'error i intenta una alternativa.
```

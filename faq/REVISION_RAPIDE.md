# 🎯 Révision Rapide - 1h pour maîtriser le code

## 📚 Programme de révision (1 heure)

### 15 min - Comprendre le flux principal

1. **Ouvrir et lire dans l'ordre :**

   ```bash
   # Terminal 1
   vim src/main.cpp
   # Comprendre : parse config → create cluster → run

   # Terminal 2
   vim src/multiplexer/EventHandler.cpp
   # Aller ligne 66 : comprendre la boucle run()
   # Aller ligne 124 : handleNewConnection()
   # Aller ligne 194 : handleClientRequest()
   ```

2. **Tracer un GET simple sur papier :**
   - Client connect → accept()
   - Client send "GET / HTTP/1.1"
   - Parse request → Read file → Send response

### 15 min - Tester et comprendre

```bash
# Terminal 1 - Lancer le serveur
./webserv config/tiny.json

# Terminal 2 - Tester
curl -v http://localhost:8080/
# Observer les logs du serveur

# Tester une erreur 404
curl http://localhost:8080/nexistepas

# Tester un upload
echo "test" > fichier.txt
curl -X POST -H "Content-Type: text/plain" --data-binary @fichier.txt http://localhost:8080/upload
```

### 15 min - Comprendre le parsing HTTP

1. **Ouvrir `src/http/Request.cpp`**

   - Ligne 16 : `parseHead()` - Comment on découpe "GET /url HTTP/1.1"
   - Comprendre : method, url, query_string

2. **Tester avec telnet pour VOIR le HTTP brut :**

   ```bash
   telnet localhost 8080
   GET / HTTP/1.1
   Host: localhost

   # (Appuyer Enter 2 fois)
   # Vous verrez la réponse HTTP complète
   ```

### 15 min - Points critiques

1. **Les 3 types d'events dans EventHandler :**

   - EP_SERVER = nouvelle connexion
   - EP_CLIENT = client qui parle
   - EP_CGI = script qui répond

2. **Les codes HTTP à retenir :**

   - 200 = OK
   - 404 = Not Found
   - 405 = Method Not Allowed (important!)

3. **Le chunked (si on vous demande) :**
   ```
   5\r\n
   Hello\r\n
   0\r\n
   \r\n
   ```

## 🔥 Les 5 fonctions à connaître

### 1. main() - `src/main.cpp`

```cpp
// Parse config JSON
// Créé Cluster
// cluster.run()
```

### 2. EventHandler::run() - `ligne 66`

```cpp
// Boucle infinie
// epoll_wait()
// Dispatch selon le type d'event
```

### 3. Request::parseHead() - `Request.cpp:16`

```cpp
// Découpe "GET /url HTTP/1.1"
// Extrait method, url, version
```

### 4. Response::parse() - `Response.cpp:79`

```cpp
// Switch sur la méthode
// handleGet/Post/Delete
```

### 5. handleClientRequest() - `EventHandler.cpp:194`

```cpp
// recv() les données
// Parse la requête
// Génère la réponse
```

## 💡 Phrases clés pour demain

- "Notre serveur est **non-bloquant** grâce à **epoll**"
- "On supporte **GET, POST et DELETE**"
- "Le parsing HTTP se fait en **3 étapes** : head, headers, body"
- "Chaque client a sa propre **ClientConnection**"
- "On gère les erreurs avec des **codes HTTP standard**"

## ✅ Test final (5 min)

1. **Où est le main ?** → src/main.cpp
2. **Où on parse les requêtes ?** → src/http/Request.cpp ligne 16
3. **Comment on gère plusieurs clients ?** → epoll dans EventHandler
4. **Quelles méthodes HTTP ?** → GET, POST, DELETE
5. **Un code d'erreur ?** → 404 Not Found

## 🚨 Si vous paniquez demain

1. Ouvrez `src/multiplexer/EventHandler.cpp`
2. Montrez la fonction `run()` ligne 66
3. Dites : "Voici notre boucle principale avec epoll"
4. Suivez le flux : Server → Client → Request → Response

**Gardez ces 3 fichiers ouverts :**

- EventHandler.cpp (le cœur)
- Request.cpp (parsing HTTP)
- Response.cpp (génération réponses)

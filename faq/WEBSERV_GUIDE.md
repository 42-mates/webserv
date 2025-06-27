# 🚀 Guide Complet du Projet Webserv - École 42

## 📋 Table des matières

1. [Vue d'ensemble du projet](#vue-densemble)
2. [Architecture du serveur](#architecture)
3. [Le protocole HTTP expliqué](#http-protocole)
4. [Implémentation du parsing HTTP](#parsing-http)
5. [Gestion des réponses HTTP](#reponses-http)
6. [Configuration du serveur](#configuration)
7. [Fonctionnalités clés](#fonctionnalites)
8. [Guide pratique pour comprendre le code](#guide-pratique)
9. [Tests et validation](#tests)

---

## 🎯 Vue d'ensemble du projet {#vue-densemble}

### Objectif

Créer un serveur HTTP/1.1 fonctionnel en C++98, compatible avec les navigateurs web réels et conforme aux RFC 7230-7235.

### Contraintes principales

- **C++98 uniquement** (pas de C++11/14/17)
- **Non-bloquant** : Un seul `select()` ou `epoll()` pour toutes les I/O
- **Pas de fork** sauf pour CGI
- **Gestion d'erreurs robuste** : Le serveur ne doit jamais crash
- **Pas de memory leaks**

### Méthodes HTTP obligatoires

- ✅ **GET** : Récupération de ressources
- ✅ **POST** : Upload de fichiers et données
- ✅ **DELETE** : Suppression de fichiers

---

## 🏗️ Architecture du serveur {#architecture}

```
┌─────────────────────────────────────────────────────────┐
│                     main.cpp                            │
│  • Parse arguments et signaux                           │
│  • Charge la configuration JSON                         │
│  • Lance le Cluster                                     │
└────────────────────────┬───────────────────────────────-┘
                         │
┌────────────────────────▼───────────────────────────────┐
│                    Cluster                             │
│  • Gère plusieurs serveurs sur différents ports        │
│  • Vérifie les conflits de ports                       │
│  • Initialise EventHandler                             │
└────────────────────────┬───────────────────────────────┘
                         │
┌────────────────────────▼───────────────────────────────┐
│                 EventHandler                           │
│  • Boucle d'événements avec epoll                      │
│  • Gère 3 types de connexions:                         │
│    - EP_SERVER : Nouvelles connexions                  │
│    - EP_CLIENT : Clients existants                     │
│    - EP_CGI : Scripts CGI                              │
└────────────────────────┬───────────────────────────────┘
                         │
     ┌───────────────────┼───────────────────┐
     │                   │                   │
┌────▼──────┐    ┌──────▼──────┐    ┌──────▼──────┐
│  Server   │    │   Client    │    │     CGI     │
│           │    │ Connection  │    │   Manager   │
└───────────┘    └──────┬──────┘    └─────────────┘
                        │
                ┌───────┴───────┐
                │               │
           ┌────▼───┐    ┌─────▼────┐
           │Request │    │ Response │
           └────────┘    └──────────┘
```

### Classes principales

| Classe               | Rôle                                | Fichiers                   |
| -------------------- | ----------------------------------- | -------------------------- |
| **Cluster**          | Gestionnaire principal des serveurs | `cluster.hpp/cpp`          |
| **EventHandler**     | Boucle d'événements (epoll)         | `EventHandler.hpp/cpp`     |
| **Server**           | Serveur écoutant sur un port        | `Server.hpp/cpp`           |
| **ClientConnection** | Connexion client active             | `ClientConnection.hpp/cpp` |
| **Request**          | Parse les requêtes HTTP             | `Request.hpp/cpp`          |
| **Response**         | Génère les réponses HTTP            | `Response.hpp/cpp`         |
| **Config**           | Configuration du serveur            | `config.hpp/cpp`           |

---

## 📡 Le protocole HTTP expliqué {#http-protocole}

### Format d'une requête HTTP

```http
GET /index.html HTTP/1.1        ← Ligne de requête
Host: localhost:8080            ← Headers
User-Agent: Mozilla/5.0
Accept: text/html
                               ← Ligne vide (CRLF)
[Corps optionnel]              ← Body (pour POST/PUT)
```

### Format d'une réponse HTTP

```http
HTTP/1.1 200 OK                ← Ligne de statut
Content-Type: text/html        ← Headers
Content-Length: 1234
Date: Mon, 01 Jan 2024 12:00:00 GMT
                               ← Ligne vide (CRLF)
<html>...</html>               ← Body
```

### Headers importants à implémenter

| Header                | Utilisation        | Exemple                               |
| --------------------- | ------------------ | ------------------------------------- |
| **Content-Length**    | Taille du body     | `Content-Length: 1234`                |
| **Content-Type**      | Type MIME          | `Content-Type: text/html`             |
| **Host**              | Serveur cible      | `Host: localhost:8080`                |
| **Transfer-Encoding** | Encodage (chunked) | `Transfer-Encoding: chunked`          |
| **Date**              | Date de la réponse | `Date: Mon, 01 Jan 2024 12:00:00 GMT` |
| **Server**            | Nom du serveur     | `Server: 3GoatServer/1.0`             |

---

## 🔍 Implémentation du parsing HTTP {#parsing-http}

### 1. Classe Request (`src/http/Request.cpp`)

```cpp
// Structure simplifiée
class Request : public AHttpData {
    std::string _method;       // GET, POST, DELETE
    std::string _url;          // /path/to/file
    std::string _http_version; // HTTP/1.1
    std::string _query_string; // ?param=value

    void parseHead(const std::string& head);
    void parseQueryString();
};
```

### 2. Processus de parsing

#### Étape 1 : Lecture de la ligne de requête

```cpp
// Exemple: "GET /index.html?name=test HTTP/1.1"
void Request::parseHead(const std::string& head) {
    // 1. Extraire la méthode
    _method = "GET";

    // 2. Extraire l'URL et query string
    _url = "/index.html";
    _query_string = "name=test";

    // 3. Vérifier la version HTTP
    if (_http_version != "HTTP/1.1")
        throw InvalidRequest("505");
}
```

#### Étape 2 : Parser les headers

```cpp
// Hérité de AHttpData
void AHttpData::parseHeaders(const std::string& headers) {
    // Parse ligne par ligne jusqu'à "\r\n\r\n"
    // Format: "Key: Value\r\n"
    _headers.insert(make_pair("Content-Type", "text/html"));
}
```

#### Étape 3 : Traiter le body

- **Content-Length** : Taille fixe du body
- **Chunked** : Body envoyé par morceaux

### 3. Gestion du Transfer-Encoding: chunked

```
POST /upload HTTP/1.1
Transfer-Encoding: chunked

7\r\n          ← Taille du chunk en hexa
Mozilla\r\n    ← Données du chunk
9\r\n          ← Taille du chunk suivant
Developer\r\n  ← Données
0\r\n          ← Fin (chunk de taille 0)
\r\n
```

**Code dans `EventHandler_chunks.cpp`** :

```cpp
void EventHandler::_decodeChunkedRequest(ClientConnection& client) {
    // Decode les chunks et reconstruit le body complet
    // Vérifie le format hexadécimal
    // Supprime les métadonnées de chunking
}
```

---

## 📤 Gestion des réponses HTTP {#reponses-http}

### 1. Classe Response (`src/http/Response.cpp`)

```cpp
class Response {
    int _statusCode;              // 200, 404, etc.
    std::string _content;         // Corps de la réponse
    std::string _contentType;     // MIME type
    Config::Route _routes;        // Configuration

    std::string generateHttpResponse();
    void handleGet(Request& req);
    void handlePost(Request& req);
    void handleDelete(Request& req);
};
```

### 2. Traitement selon la méthode

#### GET - Lecture de fichiers

```cpp
void Response::handleGet(Request& req) {
    // 1. Vérifier si c'est un répertoire
    if (isDirectory(path)) {
        if (route.dir_listing)
            generateDirectoryListing();
        else
            serveIndexFile();
    }
    // 2. Servir le fichier
    else {
        readFile(path);
        setContentType(getExtension(path));
    }
}
```

#### POST - Upload de fichiers

```cpp
void Response::handlePost(Request& req) {
    // 1. Vérifier Content-Type
    string contentType = req.getHeader("Content-Type");

    // 2. Extraire l'extension du fichier
    string extension = getFileExtension(contentType);

    // 3. Générer nom unique (timestamp)
    string filename = generateUniqueFilename(extension);

    // 4. Sauvegarder dans le dossier d'upload
    writeFile(uploadPath + filename, req.getBody());

    _statusCode = 201; // Created
}
```

#### DELETE - Suppression

```cpp
void Response::handleDelete(Request& req) {
    // 1. Vérifier que le fichier existe
    // 2. Vérifier les permissions
    // 3. Supprimer le fichier
    unlink(filePath.c_str());
    _statusCode = 204; // No Content
}
```

### 3. Codes de statut HTTP

| Code    | Signification         | Utilisation           |
| ------- | --------------------- | --------------------- |
| **200** | OK                    | Succès standard       |
| **201** | Created               | Fichier créé (POST)   |
| **204** | No Content            | Suppression réussie   |
| **301** | Moved Permanently     | Redirection           |
| **400** | Bad Request           | Requête malformée     |
| **404** | Not Found             | Fichier introuvable   |
| **405** | Method Not Allowed    | Méthode non autorisée |
| **413** | Payload Too Large     | Body trop grand       |
| **500** | Internal Server Error | Erreur serveur        |

---

## ⚙️ Configuration du serveur {#configuration}

### Format JSON (`config/config.json`)

```json
{
  "servers": [
    {
      "host": "127.0.0.1",
      "port": 8080,
      "server_names": ["localhost", "webserv.local"],
      "error_pages": {
        "404": "./error-page/not_found_404.html",
        "405": "./error-page/method_not_allowed_405.html"
      },
      "max_body_size": 10485760, // 10MB
      "routes": [
        {
          "url": "/",
          "method": ["GET", "POST"],
          "root": "./website",
          "index": "index.html",
          "dir_listing": true
        },
        {
          "url": "/upload",
          "method": ["POST", "DELETE"],
          "root": "./upload",
          "upload_path": "./upload/"
        },
        {
          "url": "/cgi-bin",
          "method": ["GET", "POST"],
          "root": "./website",
          "cgi": {
            ".py": "/usr/bin/python3",
            ".php": "/usr/bin/php"
          }
        }
      ]
    }
  ]
}
```

### Structure Config (`include/parser/config.hpp`)

```cpp
struct Route {
    std::string url;
    std::vector<std::string> methods;
    std::string root;
    std::string index;
    bool dir_listing;
    std::string upload_path;
    std::map<std::string, std::string> cgi;
    std::string redirect_url;
    int redirect_code;
};

class Config {
    std::vector<Route> routes;
    std::map<int, std::string> error_pages;
    size_t max_body_size;
    // ...
};
```

---

## 🔧 Fonctionnalités clés {#fonctionnalites}

### 1. Serveur non-bloquant avec epoll

```cpp
// EventHandler.cpp
void EventHandler::run() {
    while (_running) {
        // Attendre des événements
        int nfds = epoll_wait(_epfd, _events, MAX_EVENTS, -1);

        for (int i = 0; i < nfds; i++) {
            if (type == EP_SERVER)
                handleNewConnection();  // Nouvelle connexion
            else if (type == EP_CLIENT)
                handleClientRequest();  // Requête client
            else if (type == EP_CGI)
                handleCgiResponse();    // Réponse CGI
        }
    }
}
```

### 2. CGI (Common Gateway Interface)

**Variables d'environnement passées au CGI** :

- `REQUEST_METHOD` : GET, POST, etc.
- `QUERY_STRING` : Paramètres après ?
- `CONTENT_LENGTH` : Taille du body
- `PATH_INFO` : Chemin du script
- `SERVER_NAME`, `SERVER_PORT`

**Exécution** :

```cpp
// CGIManager.cpp
void CGIManager::executeCgi() {
    // 1. Fork un processus enfant
    pid_t pid = fork();

    if (pid == 0) {
        // 2. Rediriger stdin/stdout
        dup2(pipe_in, STDIN_FILENO);
        dup2(pipe_out, STDOUT_FILENO);

        // 3. Préparer l'environnement
        setenv("REQUEST_METHOD", method.c_str(), 1);

        // 4. Exécuter le script
        execve(interpreter, args, env);
    }
}
```

### 3. Directory Listing

Génère une page HTML avec :

- Liste des fichiers/dossiers
- Icônes selon le type
- Navigation cliquable
- Style CSS intégré

### 4. Upload de fichiers

Supporte :

- Simple POST avec Content-Type
- Multipart/form-data
- Validation des types MIME
- Génération de noms uniques

---

## 📚 Guide pratique pour comprendre le code {#guide-pratique}

### 1. Points d'entrée importants

| Fichier                               | Description         | Commencer ici pour      |
| ------------------------------------- | ------------------- | ----------------------- |
| `src/main.cpp`                        | Point d'entrée      | Comprendre le démarrage |
| `src/multiplexer/EventHandler.cpp:66` | Boucle principale   | Comprendre le flux      |
| `src/http/Request.cpp:16`             | Parsing requêtes    | Comprendre HTTP         |
| `src/http/Response.cpp:79`            | Génération réponses | Comprendre les réponses |

### 2. Flux d'une requête HTTP

```
1. Client se connecte
   └─> EventHandler::handleNewConnection() [EventHandler.cpp:124]

2. Client envoie requête
   └─> EventHandler::handleClientRequest() [EventHandler.cpp:194]
       └─> Request::parse() [Request.cpp:16]

3. Serveur traite la requête
   └─> Response::parse() [Response.cpp:79]
       └─> handleGet/Post/Delete()

4. Serveur envoie la réponse
   └─> EventHandler::handleResponse() [EventHandler.cpp:334]
```

### 3. Debugging tips

```cpp
// Activer les logs détaillés
#define DEBUG 1  // Dans utils.hpp

// Points de debug utiles :
// - Request::parseHead() : Voir la requête brute
// - Response::generateHttpResponse() : Voir la réponse générée
// - EventHandler::handleClientRequest() : Suivre le flux
```

---

## 🧪 Tests et validation {#tests}

### 1. Tests basiques avec telnet

```bash
# Test GET
telnet localhost 8080
GET / HTTP/1.1
Host: localhost
[Entrée 2x]

# Test POST
telnet localhost 8080
POST /upload HTTP/1.1
Host: localhost
Content-Type: text/plain
Content-Length: 11

Hello World
```

### 2. Tests avec curl

```bash
# GET simple
curl http://localhost:8080/

# POST avec fichier
curl -X POST -H "Content-Type: image/jpeg" \
     --data-binary @image.jpg \
     http://localhost:8080/upload

# DELETE
curl -X DELETE http://localhost:8080/upload/file.txt
```

### 3. Tests Hurl (fournis)

```bash
cd test/
./hurl config.hurl    # Test configuration
./hurl upload.hurl    # Test upload
./hurl cookies.hurl   # Test cookies/CGI
```

### 4. Validation importante

- ✅ Le serveur ne doit **jamais crash**
- ✅ Pas de **memory leaks** (valgrind)
- ✅ Gestion correcte des **déconnexions client**
- ✅ Support des **requêtes simultanées**
- ✅ **Timeouts** fonctionnels
- ✅ **Chunked encoding** correct
- ✅ **CGI** avec Python/PHP

---

## 💡 Conseils pour réussir

1. **Comprendre d'abord** : Lisez le code dans l'ordre du flux
2. **Tester souvent** : Utilisez telnet pour des tests manuels
3. **Comparer avec nginx** : En cas de doute sur un comportement
4. **Logger intelligemment** : Ajoutez des prints aux endroits clés
5. **Gérer les erreurs** : Toujours prévoir les cas d'erreur

## 🚀 Commandes utiles

```bash
# Compiler
make

# Lancer avec config par défaut
./webserv

# Lancer avec config custom
./webserv config/tiny.json

# Tester
curl -I http://localhost:8080/  # Headers only
curl -v http://localhost:8080/  # Verbose

# Stress test
ab -n 1000 -c 10 http://localhost:8080/  # Apache Bench
```

---

# 📋 FAQ Webserv - Questions & Réponses Détaillées

## 📌 Table des matières

1. [Questions sur les Events](#events)
2. [Questions sur CGI](#cgi)
3. [Questions sur le Parsing HTTP](#parsing-http)
4. [Questions sur les Concepts HTTP](#concepts-http)
5. [Architecture et Code Clean](#architecture-clean)

---

## 🎯 Questions sur les Events {#events}

### Q: Les events, elles servent à quoi ? C'est quoi exactement ?

**Réponse détaillée :**

Les **events** (événements) sont le cœur du serveur non-bloquant. C'est un mécanisme qui permet au serveur de gérer plusieurs connexions simultanément sans bloquer.

**Concept de base :**
```cpp
// Au lieu de faire ça (bloquant) :
int data = read(socket);  // ❌ Bloque jusqu'à recevoir des données

// On fait ça (non-bloquant) :
epoll_wait(events);  // ✅ Attend que quelque chose se passe sur N'IMPORTE QUEL socket
```

**Les 3 types d'events dans votre serveur :**

1. **EP_SERVER** : Nouvelle connexion entrante
   ```cpp
   // Un client veut se connecter
   Event: "Hey, quelqu'un frappe à la porte !"
   Action: accept() la connexion
   ```

2. **EP_CLIENT** : Un client existant envoie des données
   ```cpp
   // Un client envoie une requête HTTP
   Event: "Le client #42 a envoyé des données"
   Action: read() les données et parser la requête
   ```

3. **EP_CGI** : Un script CGI a fini/envoyé des données
   ```cpp
   // Le script Python a fini de s'exécuter
   Event: "Le CGI a des données à lire"
   Action: read() la sortie du CGI
   ```

**Exemple concret :**
```cpp
void EventHandler::run() {
    while (_running) {
        // "Dis-moi qui a quelque chose d'intéressant"
        int nfds = epoll_wait(_epfd, _events, MAX_EVENTS, -1);
        
        // Pour chaque event
        for (int i = 0; i < nfds; i++) {
            if (type == EP_SERVER)
                // "Nouveau client qui veut se connecter"
                handleNewConnection();
            else if (type == EP_CLIENT)  
                // "Client existant qui envoie une requête"
                handleClientRequest();
            else if (type == EP_CGI)
                // "Script CGI qui a fini"
                handleCgiResponse();
        }
    }
}
```

### Q: Les events envoient à la fois serveur, client et CGI ?

**Réponse :**

Non, les events ne "envoient" pas. Les events **surveillent** et **notifient**.

- **epoll** surveille TOUS les file descriptors (sockets) en même temps
- Quand quelque chose se passe sur un socket, epoll le signale
- Le code vérifie alors quel TYPE de socket a généré l'event

```cpp
// Exemple : 3 sockets surveillés
Socket serveur (fd=3) : écoute sur le port 8080
Socket client1 (fd=4) : connexion active avec Chrome  
Socket CGI     (fd=5) : pipe vers script Python

epoll_wait() retourne → "fd=4 a des données"
→ On sait que c'est un client → handleClientRequest()
```

---

## 🔧 Questions sur CGI {#cgi}

### Q: C'est quoi les CGI ?

**CGI = Common Gateway Interface**

C'est une méthode pour faire exécuter des programmes externes (Python, PHP, etc.) par le serveur web et renvoyer leur résultat au client.

**Exemple simple :**
1. Client demande : `GET /hello.py`
2. Serveur voit `.py` → "C'est un CGI !"
3. Serveur exécute : `python3 hello.py`
4. Script affiche : `print("Hello World")`
5. Serveur récupère la sortie et l'envoie au client

**Dans le code :**
```cpp
// CGIManager.cpp
void executeCgi() {
    // 1. Créer des pipes pour communiquer
    pipe(pipe_in);   // Pour envoyer des données AU script
    pipe(pipe_out);  // Pour recevoir des données DU script
    
    // 2. Fork (créer un processus enfant)
    if (fork() == 0) {
        // Enfant : devient le script
        dup2(pipe_out[1], STDOUT_FILENO);  // stdout → pipe
        execve("/usr/bin/python3", ["python3", "script.py"], env);
    } else {
        // Parent : lit la sortie du script
        read(pipe_out[0], buffer, size);
    }
}
```

**Variables d'environnement CGI :**
```bash
REQUEST_METHOD=POST          # Méthode HTTP
QUERY_STRING=name=john&age=25  # Paramètres URL
CONTENT_LENGTH=1234          # Taille du body
PATH_INFO=/hello.py          # Chemin du script
```

---

## 📝 Questions sur le Parsing HTTP {#parsing-http}

### Q: Pourquoi Request return AHttpData ?

**Réponse :**

`Request` **hérite** de `AHttpData`, elle ne le "retourne" pas.

```cpp
class AHttpData {  // Classe abstraite (A = Abstract)
    // Contient les parties communes HTTP
    std::multimap<string, string> _headers;
    std::string _body;
    
    void parseHeaders();  // Méthode commune
};

class Request : public AHttpData {  // Hérite
    // Ajoute les parties spécifiques aux requêtes
    std::string _method;     // GET, POST...
    std::string _url;        // /index.html
    std::string _query_string; // ?param=value
};
```

**Pourquoi cette architecture ?**
- `AHttpData` contient ce qui est COMMUN aux requêtes ET réponses (headers, body)
- `Request` ajoute ce qui est SPÉCIFIQUE aux requêtes (method, URL)
- `Response` pourrait aussi hériter et ajouter ses spécificités

### Q: Que fait la méthode parseHead ?

**parseHead** analyse la première ligne d'une requête HTTP :

```http
GET /index.html?search=webserv HTTP/1.1
↑    ↑                          ↑
méthode  URL + query string     version
```

**Code détaillé :**
```cpp
void Request::parseHead(const std::string& head) {
    // Exemple : "GET /index.html?search=webserv HTTP/1.1"
    
    // 1. Trouver les espaces qui séparent les 3 parties
    size_t space1 = head.find(' ');           // Position après GET
    size_t space2 = head.find(' ', space1+1); // Position après URL
    
    // 2. Extraire la méthode
    _method = head.substr(0, space1);  // "GET"
    
    // 3. Extraire l'URL complète
    string full_url = head.substr(space1+1, space2-space1-1);
    // full_url = "/index.html?search=webserv"
    
    // 4. Séparer URL et query string
    size_t question = full_url.find('?');
    if (question != string::npos) {
        _url = full_url.substr(0, question);    // "/index.html"
        _query_string = full_url.substr(question+1); // "search=webserv"
    }
    
    // 5. Extraire et vérifier la version
    _http_version = head.substr(space2+1);  // "HTTP/1.1"
    if (_http_version != "HTTP/1.1")
        throw InvalidRequest("505");  // Version non supportée
}
```

### Q: Les query strings c'est les params, c'est ça ?

**OUI ! Exactement.**

Query string = les paramètres dans l'URL après le `?`

**Exemples :**
```
https://google.com/search?q=webserv&lang=fr
                          ↑________________↑
                          query string

Décomposé :
- q = webserv       (paramètre de recherche)
- lang = fr         (paramètre de langue)
```

**Dans le code :**
```cpp
// URL : /form.php?name=John&age=25&city=Paris

_url = "/form.php"
_query_string = "name=John&age=25&city=Paris"

// Le CGI recevra :
// QUERY_STRING="name=John&age=25&city=Paris"
```

### Q: parseHead et parseHeaders c'est différent ?

**OUI, totalement différent !**

1. **parseHead** (première ligne uniquement)
   ```http
   GET /index.html HTTP/1.1  ← Seulement cette ligne
   ```

2. **parseHeaders** (toutes les lignes suivantes)
   ```http
   Host: localhost:8080       ← Ces
   User-Agent: Mozilla/5.0    ← lignes
   Content-Type: text/html    ← là
   Content-Length: 1234       ← 
   ```

**Ordre chronologique :**
```cpp
// 1. D'abord parseHead
parseHead("GET /index.html HTTP/1.1");

// 2. Ensuite parseHeaders  
parseHeaders("Host: localhost\r\n"
            "User-Agent: Mozilla\r\n"
            "Content-Length: 42\r\n");

// 3. Enfin le body (si Content-Length > 0)
_body = "Les données du body...";
```

### Q: Comment le body est traité ? Exemples ?

**Traitement du body - 3 cas :**

#### 1. **Body avec Content-Length** (cas normal)
```http
POST /upload HTTP/1.1
Content-Length: 11

Hello World
```

**Code :**
```cpp
void Request::parseBody(const string& raw_request) {
    // 1. Trouver la fin des headers (\r\n\r\n)
    size_t headers_end = raw_request.find("\r\n\r\n");
    
    // 2. Récupérer Content-Length
    int content_length = atoi(getHeader("Content-Length").c_str());
    
    // 3. Extraire le body
    _body = raw_request.substr(headers_end + 4, content_length);
    // _body = "Hello World"
}
```

#### 2. **Body chunked** (Transfer-Encoding: chunked)
```http
POST /upload HTTP/1.1
Transfer-Encoding: chunked

5\r\n      ← Taille en hexa (5 octets)
Hello\r\n  ← Données
6\r\n      ← Taille suivante (6 octets)  
World!\r\n ← Données
0\r\n      ← Fin (taille 0)
\r\n
```

**Code détaillé :**
```cpp
void EventHandler::_decodeChunkedRequest(ClientConnection& client) {
    string decoded_body;
    string raw = client.getBuffer();
    size_t pos = 0;
    
    while (true) {
        // 1. Lire la taille du chunk (en hexa)
        size_t size_end = raw.find("\r\n", pos);
        string size_hex = raw.substr(pos, size_end - pos);
        int chunk_size = strtol(size_hex.c_str(), NULL, 16);
        
        // 2. Si taille = 0, c'est fini
        if (chunk_size == 0) break;
        
        // 3. Lire les données du chunk
        pos = size_end + 2;  // Skip \r\n
        decoded_body += raw.substr(pos, chunk_size);
        pos += chunk_size + 2;  // Skip data + \r\n
    }
    
    client.setBody(decoded_body);  // "HelloWorld!"
}
```

#### 3. **Body multipart** (upload de fichiers)
```http
POST /upload HTTP/1.1
Content-Type: multipart/form-data; boundary=----ABC123

------ABC123
Content-Disposition: form-data; name="file"; filename="photo.jpg"
Content-Type: image/jpeg

[Données binaires de l'image]
------ABC123--
```

---

## 🌐 Questions sur les Concepts HTTP {#concepts-http}

### Q: C'est quoi le MIME type ?

**MIME = Multipurpose Internet Mail Extensions**

C'est une façon de dire "quel type de fichier c'est" dans HTTP.

**Format :** `type/sous-type`

**Exemples courants :**
```
text/html           → Page HTML
text/css            → Fichier CSS
text/plain          → Texte simple
image/jpeg          → Image JPG
image/png           → Image PNG
application/json    → Données JSON
application/pdf     → Document PDF
video/mp4           → Vidéo MP4
```

**Dans le code :**
```cpp
// Déterminer le MIME type selon l'extension
std::string getMimeType(const std::string& path) {
    if (path.ends_with(".html")) return "text/html";
    if (path.ends_with(".jpg"))  return "image/jpeg";
    if (path.ends_with(".json")) return "application/json";
    // etc...
}

// Utilisation dans Response
response.setHeader("Content-Type", getMimeType("/photo.jpg"));
// → "Content-Type: image/jpeg"
```

**Pourquoi c'est important ?**
- Le navigateur sait comment afficher le fichier
- Sécurité : empêche l'exécution de fichiers dangereux
- Upload : vérifie le type de fichier envoyé

---

## 🏗️ Architecture et Code Clean {#architecture-clean}

### Q: Comment mieux architecturer le code sans tout casser ?

**Voici les améliorations possibles SANS risquer la soutenance :**

#### 1. **Ajouter des commentaires en anglais**

```cpp
// AVANT
void EventHandler::handleClientRequest(EventData* event_data) {
    ClientConnection* client = static_cast<ClientConnection*>(event_data);
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    
    int bytes = recv(client->getSocket(), buffer, BUFFER_SIZE - 1, 0);

// APRÈS
void EventHandler::handleClientRequest(EventData* event_data) {
    ClientConnection* client = static_cast<ClientConnection*>(event_data);
    
    // Initialize buffer for incoming data
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    
    // Read data from client socket (non-blocking)
    int bytes = recv(client->getSocket(), buffer, BUFFER_SIZE - 1, 0);
```

#### 2. **Constantes et macros plus claires**

```cpp
// include/utils.hpp - Ajouter au début
// Buffer sizes
#define BUFFER_SIZE 8192
#define MAX_EVENTS 64
#define CGI_TIMEOUT 5000  // 5 seconds in milliseconds

// HTTP constants
#define HTTP_VERSION "HTTP/1.1"
#define SERVER_NAME "3GoatServer/1.0"
#define CRLF "\r\n"
#define DOUBLE_CRLF "\r\n\r\n"

// Status messages
#define MSG_200_OK "OK"
#define MSG_201_CREATED "Created"
#define MSG_204_NO_CONTENT "No Content"
#define MSG_400_BAD_REQUEST "Bad Request"
#define MSG_404_NOT_FOUND "Not Found"
```

#### 3. **Fonctions helper pour clarifier**

```cpp
// Dans Response.cpp - Ajouter des helpers
private:
    // Check if path is within allowed directory (security)
    bool isPathSafe(const std::string& path) const {
        return path.find("..") == std::string::npos;
    }
    
    // Generate timestamp for unique filenames
    std::string generateTimestamp() const {
        char buffer[20];
        time_t now = time(0);
        strftime(buffer, 20, "%Y%m%d_%H%M%S", localtime(&now));
        return std::string(buffer);
    }
    
    // Build full HTTP response with headers
    std::string buildHttpResponse(int code, const std::string& body) const {
        std::stringstream response;
        response << HTTP_VERSION << " " << code << " " << getStatusMessage(code) << CRLF;
        response << "Server: " << SERVER_NAME << CRLF;
        response << "Date: " << getCurrentDate() << CRLF;
        response << "Content-Length: " << body.length() << CRLF;
        response << "Content-Type: " << _contentType << CRLF;
        response << CRLF;
        response << body;
        return response.str();
    }
```

#### 4. **Validation et sécurité**

```cpp
// Dans Request.cpp - Ajouter des validations
void Request::parse(const std::string& raw) {
    // Security: Check max request size
    if (raw.length() > MAX_REQUEST_SIZE) {
        throw InvalidRequest("413");  // Payload Too Large
    }
    
    // Security: Validate HTTP method
    if (_method != "GET" && _method != "POST" && _method != "DELETE") {
        throw InvalidRequest("405");  // Method Not Allowed
    }
    
    // Security: Validate URL characters
    if (_url.find_first_of("\r\n") != std::string::npos) {
        throw InvalidRequest("400");  // Bad Request
    }
}
```

#### 5. **Organisation des includes**

```cpp
// Au début de chaque fichier .cpp
// System includes
#include <iostream>
#include <string>
#include <vector>
#include <map>

// Project includes  
#include "utils.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"

// Class header
#include "multiplexer/EventHandler.hpp"
```

#### 6. **Messages d'erreur plus clairs**

```cpp
// utils.cpp - Améliorer les messages
void exitWithError(const std::string& context, int code = 1) {
    std::cerr << RED << "[ERROR] " << context;
    if (errno != 0) {
        std::cerr << ": " << strerror(errno);
    }
    std::cerr << RESET << std::endl;
    exit(code);
}

// Utilisation
exitWithError("Failed to bind socket on port " + toString(port));
// Au lieu de
exitWithError("bind");
```

### Conseils pour demain :

1. **NE PAS** refactoriser les classes principales
2. **NE PAS** changer la logique métier
3. **JUSTE** ajouter des commentaires explicatifs
4. **TESTER** après chaque modification
5. **COMMIT** avant de commencer les changements

```bash
# Avant de toucher au code
git add .
git commit -m "Pre-evaluation backup"
git push

# Créer une branche de sauvegarde
git checkout -b pre-eval-backup
git checkout main
```

**Priorités pour améliorer sans risque :**
1. ✅ Commentaires en anglais sur les parties complexes
2. ✅ Constantes nommées au lieu de valeurs magiques
3. ✅ Messages d'erreur plus descriptifs
4. ❌ PAS de refactoring de structure
5. ❌ PAS de changement d'architecture

Le code fonctionne bien, il répond aux exigences. L'important est qu'il soit compréhensible pour l'évaluateur.
#!/bin/bash

# ============================================================================
# WEBSERV TESTS - École 42 
# ============================================================================
# Script de tests complet pour le serveur HTTP webserv
# Remplacez <URL> par l'URL de votre serveur (ex: localhost:8080)
# ============================================================================

# Configuration par défaut
BASE_URL="<URL>"
SERVER1="localhost:8080"
SERVER2="localhost:8081"
SERVER3="localhost:9999"

# Couleurs pour l'affichage
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo_test() {
    echo -e "${BLUE}=== $1 ===${NC}"
}

echo_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

echo_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

echo_error() {
    echo -e "${RED}✗ $1${NC}"
}

# ============================================================================
# TESTS BASIQUES - SERVEUR 1 (PORT 8080)
# ============================================================================

echo_test "TESTS BASIQUES - SERVEUR 1 (PORT 8080)"

# Test 1: GET page d'accueil
echo_test "Test 1: GET page d'accueil"
curl -X GET http://${SERVER1}/ -v

# Test 2: GET avec headers spécifiques
echo_test "Test 2: GET avec headers HTTP"
curl -X GET http://${SERVER1}/ \
    -H "User-Agent: webserv-test/1.0" \
    -H "Accept: text/html,application/xhtml+xml" \
    -H "Accept-Language: fr-FR,fr;q=0.9,en;q=0.8" \
    -H "Accept-Charset: utf-8" \
    -v

# Test 3: GET d'une page inexistante (404)
echo_test "Test 3: GET page inexistante (404)"
curl -X GET http://${SERVER1}/nonexistent.html -v

# Test 4: Directory listing
echo_test "Test 4: Directory listing"
curl -X GET http://${SERVER1}/first/ -v

# Test 5: Fichier spécifique
echo_test "Test 5: Fichier spécifique"
curl -X GET http://${SERVER1}/index.html -v

# ============================================================================
# TESTS MÉTHODES HTTP
# ============================================================================

echo_test "TESTS MÉTHODES HTTP"

# Test 6: POST simple
echo_test "Test 6: POST simple"
curl -X POST http://${SERVER1}/ \
    -H "Content-Type: text/plain" \
    -d "Hello World from POST" \
    -v

# Test 7: POST avec form data
echo_test "Test 7: POST avec form data"
curl -X POST http://${SERVER1}/form/ \
    -H "Content-Type: application/x-www-form-urlencoded" \
    -d "name=John&email=john@example.com&message=Test message" \
    -v

# Test 8: DELETE méthode
echo_test "Test 8: DELETE méthode"
curl -X DELETE http://${SERVER1}/test.txt -v

# Test 9: Méthode non autorisée (PUT)
echo_test "Test 9: Méthode non autorisée (PUT)"
curl -X PUT http://${SERVER1}/ \
    -H "Content-Type: text/plain" \
    -d "This should fail" \
    -v

# ============================================================================
# TESTS UPLOAD DE FICHIERS
# ============================================================================

echo_test "TESTS UPLOAD DE FICHIERS"

# Test 10: Upload fichier texte
echo_test "Test 10: Upload fichier texte"
echo "Contenu de test pour upload" > /tmp/test_upload.txt
curl -X POST http://${SERVER1}/ \
    -H "Content-Type: multipart/form-data" \
    -F "file=@/tmp/test_upload.txt" \
    -v

# Test 11: Upload fichier avec nom spécifique
echo_test "Test 11: Upload avec nom spécifique"
curl -X POST http://${SERVER1}/ \
    -H "Content-Type: multipart/form-data" \
    -F "file=@/tmp/test_upload.txt;filename=mon_fichier.txt" \
    -v

# Test 12: Upload fichier trop gros (dépasse max_body)
echo_test "Test 12: Upload fichier trop gros"
dd if=/dev/zero of=/tmp/big_file.txt bs=1M count=20 2>/dev/null
curl -X POST http://${SERVER1}/ \
    -H "Content-Type: multipart/form-data" \
    -F "file=@/tmp/big_file.txt" \
    -v

# ============================================================================
# TESTS CGI - PYTHON
# ============================================================================

echo_test "TESTS CGI - PYTHON"

# Test 13: CGI Python simple
echo_test "Test 13: CGI Python simple"
curl -X GET http://${SERVER1}/hello.py -v

# Test 14: CGI Python avec paramètres GET
echo_test "Test 14: CGI Python avec paramètres GET"
curl -X GET "http://${SERVER1}/env.py?name=test&value=123" -v

# Test 15: CGI Python avec POST
echo_test "Test 15: CGI Python avec POST"
curl -X POST http://${SERVER1}/hello.py \
    -H "Content-Type: application/x-www-form-urlencoded" \
    -d "name=WebServ&action=test" \
    -v

# ============================================================================
# TESTS CGI - PHP
# ============================================================================

echo_test "TESTS CGI - PHP"

# Test 16: CGI PHP simple
echo_test "Test 16: CGI PHP simple"
curl -X GET http://${SERVER1}/first/hello.php -v

# Test 17: CGI PHP avec paramètres
echo_test "Test 17: CGI PHP avec paramètres"
curl -X GET "http://${SERVER1}/first/hello.php?name=WebServ&test=42" -v

# ============================================================================
# TESTS REDIRECTION
# ============================================================================

echo_test "TESTS REDIRECTION"

# Test 18: Redirection (sans suivre)
echo_test "Test 18: Redirection (sans suivre)"
curl -X GET http://${SERVER1}/redirect -v

# Test 19: Redirection (en suivant)
echo_test "Test 19: Redirection (en suivant)"
curl -X GET http://${SERVER1}/redirect -L -v

# ============================================================================
# TESTS HEADERS HTTP AVANCÉS
# ============================================================================

echo_test "TESTS HEADERS HTTP AVANCÉS"

# Test 20: Headers d'autorisation
echo_test "Test 20: Headers d'autorisation"
curl -X GET http://${SERVER1}/ \
    -H "Authorization: Basic dGVzdDp0ZXN0" \
    -v

# Test 21: Headers de contenu
echo_test "Test 21: Headers de contenu avancés"
curl -X POST http://${SERVER1}/ \
    -H "Content-Type: application/json" \
    -H "Content-Language: fr-FR" \
    -H "User-Agent: webserv-advanced-test/2.0" \
    -d '{"name": "test", "value": 42}' \
    -v

# Test 22: Headers de cache
echo_test "Test 22: Headers de cache"
curl -X GET http://${SERVER1}/index.html \
    -H "If-Modified-Since: Wed, 21 Oct 2015 07:28:00 GMT" \
    -v

# ============================================================================
# TESTS SERVEUR 2 (PORT 8081)
# ============================================================================

echo_test "TESTS SERVEUR 2 (PORT 8081)"

# Test 23: GET serveur 2
echo_test "Test 23: GET serveur 2"
curl -X GET http://${SERVER2}/ -v

# Test 24: POST serveur 2
echo_test "Test 24: POST serveur 2"
curl -X POST http://${SERVER2}/ \
    -H "Content-Type: text/plain" \
    -d "Test sur serveur 2" \
    -v

# Test 25: DELETE non autorisé sur serveur 2
echo_test "Test 25: DELETE non autorisé sur serveur 2"
curl -X DELETE http://${SERVER2}/test.txt -v

# ============================================================================
# TESTS SERVEUR 3 (PORT 9999)
# ============================================================================

echo_test "TESTS SERVEUR 3 (PORT 9999)"

# Test 26: GET serveur 3
echo_test "Test 26: GET serveur 3"
curl -X GET http://${SERVER3}/ -v

# Test 27: CGI Python sur serveur 3
echo_test "Test 27: CGI Python sur serveur 3"
curl -X GET http://${SERVER3}/hello.py -v

# ============================================================================
# TESTS DE CHARGE ET LIMITES
# ============================================================================

echo_test "TESTS DE CHARGE ET LIMITES"

# Test 28: Multiples requêtes simultanées
echo_test "Test 28: Multiples requêtes simultanées"
for i in {1..10}; do
    curl -X GET http://${SERVER1}/ -s -o /dev/null &
done
wait

# Test 29: Requête avec body de taille limite
echo_test "Test 29: Requête avec body de taille limite"
curl -X POST http://${SERVER2}/ \
    -H "Content-Type: text/plain" \
    -d "$(head -c 1999 /dev/zero | tr '\0' 'A')" \
    -v

# ============================================================================
# TESTS CHUNKED TRANSFER ENCODING
# ============================================================================

echo_test "TESTS CHUNKED TRANSFER ENCODING"

# Test 30: POST avec Transfer-Encoding chunked
echo_test "Test 30: POST avec Transfer-Encoding chunked"
curl -X POST http://${SERVER1}/ \
    -H "Transfer-Encoding: chunked" \
    -H "Content-Type: text/plain" \
    -d "Données en chunks" \
    -v

# ============================================================================
# TESTS COOKIES ET SESSIONS
# ============================================================================

echo_test "TESTS COOKIES ET SESSIONS"

# Test 31: Envoi de cookies
echo_test "Test 31: Envoi de cookies"
curl -X GET http://${SERVER1}/cookies/ \
    -H "Cookie: session_id=abc123; user=webserv" \
    -v

# Test 32: Réception de cookies
echo_test "Test 32: Test réception de cookies"
curl -X GET http://${SERVER1}/cookies/ -c /tmp/cookies.txt -v

# ============================================================================
# TESTS SPÉCIAUX ET EDGE CASES
# ============================================================================

echo_test "TESTS SPÉCIAUX ET EDGE CASES"

# Test 33: Caractères spéciaux dans URL
echo_test "Test 33: Caractères spéciaux dans URL"
curl -X GET "http://${SERVER1}/test%20file.html" -v

# Test 34: URL très longue
echo_test "Test 34: URL très longue"
long_path=$(printf 'a%.0s' {1..1000})
curl -X GET "http://${SERVER1}/${long_path}" -v

# Test 35: Headers très longs
echo_test "Test 35: Headers très longs"
long_header=$(printf 'a%.0s' {1..1000})
curl -X GET http://${SERVER1}/ \
    -H "X-Long-Header: ${long_header}" \
    -v

# Test 36: Requête malformée
echo_test "Test 36: Test connection avec telnet (requête malformée)"
echo -e "GET / HTTP/1.1\nHost: ${SERVER1}\nConnection: close\n\nGET /invalid" | nc localhost 8080

# ============================================================================
# TESTS TIMEOUT
# ============================================================================

echo_test "TESTS TIMEOUT"

# Test 37: Timeout CGI
echo_test "Test 37: Timeout CGI"
curl -X GET http://${SERVER1}/timeout.py --max-time 30 -v

# ============================================================================
# NETTOYAGE
# ============================================================================

echo_test "NETTOYAGE"
rm -f /tmp/test_upload.txt /tmp/big_file.txt /tmp/cookies.txt

echo_success "Tous les tests sont terminés!"
echo_warning "Vérifiez les codes de retour et les réponses pour valider le comportement de votre serveur."
echo_warning "Adaptez les URLs selon votre configuration (remplacez <URL> par localhost:8080, etc.)"

# ============================================================================
# COMMANDES CURL UTILES SUPPLÉMENTAIRES
# ============================================================================

echo_test "COMMANDES CURL UTILES SUPPLÉMENTAIRES"

cat << 'EOF'

# Autres commandes utiles pour tester votre serveur :

# Test avec des headers personnalisés
curl -X GET <URL> -H "X-Custom-Header: test-value" -v

# Test avec authentification
curl -X GET <URL> --user username:password -v

# Test de performance (temps de réponse)
curl -X GET <URL> -w "@curl-format.txt" -o /dev/null -s

# Test avec proxy
curl -X GET <URL> --proxy http://proxy:port -v

# Test avec certificat SSL (si HTTPS)
curl -X GET <URL> --insecure -v

# Test avec compression
curl -X GET <URL> -H "Accept-Encoding: gzip, deflate" -v

# Sauvegarde de la réponse
curl -X GET <URL> -o response.html -v

# Test avec plusieurs cookies
curl -X GET <URL> -b "cookie1=value1; cookie2=value2" -v

# Test POST avec fichier JSON
curl -X POST <URL> -H "Content-Type: application/json" -d @data.json -v

# Test avec méthode TRACE (si supportée)
curl -X TRACE <URL> -v

# Test avec méthode OPTIONS
curl -X OPTIONS <URL> -v

# Test avec Range header (téléchargement partiel)
curl -X GET <URL> -H "Range: bytes=0-1023" -v

EOF
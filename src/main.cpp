// Palestra digitale - entry point del backend.
// Caso di studio ITPS, traccia 2.

#include <cstdio>

#include "httplib.h"

// Indirizzo e porta su cui il server resta in ascolto.
// "0.0.0.0" significa "ascolta su qualsiasi interfaccia di rete".
const char* INDIRIZZO = "0.0.0.0";
const int PORTA = 8080;

int main() {
    httplib::Server server;

    // I blocchi [](...) { ... } qui sotto sono "lambda": funzioni anonime
    // che cpp-httplib chiama ogni volta che arriva una richiesta HTTP
    // all'URL indicato. In pratica sono dei callback in stile C, scritti
    // in modo piu' compatto.
    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(
            "<!doctype html><meta charset='utf-8'>"
            "<title>Palestra digitale</title>"
            "<h1>Palestra digitale</h1>"
            "<p>Server attivo. Lo step 0 e' stato completato.</p>",
            "text/html; charset=utf-8");
    });

    server.Get("/api/ping", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("{\"ok\":true,\"messaggio\":\"pong\"}", "application/json");
    });

    printf("Palestra digitale - server in ascolto su http://localhost:%d\n", PORTA);
    printf("Premi Ctrl+C per terminare.\n");

    if (!server.listen(INDIRIZZO, PORTA)) {
        fprintf(stderr, "Errore: impossibile avviare il server sulla porta %d.\n", PORTA);
        return 1;
    }
    return 0;
}

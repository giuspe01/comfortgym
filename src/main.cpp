// Palestra digitale - entry point del backend.
// Caso di studio ITPS, traccia 2.

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "httplib.h"

#include "persistenza/Database.h"
#include "controller/AuthController.h"
#include "controller/ProgrammiController.h"
#include "controller/PianiController.h"

// Indirizzo e porta su cui il server resta in ascolto.
// "0.0.0.0" significa "ascolta su qualsiasi interfaccia di rete".
const char* INDIRIZZO = "0.0.0.0";
const int PORTA = 8080;

// Percorso del file SQLite. La cartella 'data' deve esistere.
const char* PERCORSO_DB = "data/palestra.db";

int main() {
    // Inizializza il generatore di numeri casuali (usato per i token di sessione).
    srand((unsigned int)time(NULL));

    // Apertura del database (e creazione automatica dello schema se serve).
    if (!apriDatabase(PERCORSO_DB)) {
        fprintf(stderr, "Impossibile aprire il database. Esco.\n");
        return 1;
    }
    printf("Database aperto: %s\n", PERCORSO_DB);

    httplib::Server server;

    // Serviamo i file statici (HTML, CSS, JS, immagini) dalla cartella 'public'.
    // Quando il browser chiede '/' arriva 'public/index.html' di default.
    if (!server.set_mount_point("/", "./public")) {
        fprintf(stderr, "Errore: cartella 'public' non trovata.\n");
        chiudiDatabase();
        return 1;
    }

    // I blocchi [](...) { ... } qui sotto sono "lambda": funzioni anonime
    // che cpp-httplib chiama ogni volta che arriva una richiesta HTTP
    // all'URL indicato. In pratica sono dei callback in stile C, scritti
    // in modo piu' compatto.
    server.Get("/api/ping", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("{\"ok\":true,\"messaggio\":\"pong\"}", "application/json");
    });

    // Rotte di autenticazione (registrazione, login, logout, profilo).
    registraRotteAuth(server);

    // Rotte dei programmi di allenamento.
    registraRotteProgrammi(server);

    // Rotte dei piani nutrizionali.
    registraRottePiani(server);

    printf("Palestra digitale - server in ascolto su http://localhost:%d\n", PORTA);
    printf("Premi Ctrl+C per terminare.\n");

    if (!server.listen(INDIRIZZO, PORTA)) {
        fprintf(stderr, "Errore: impossibile avviare il server sulla porta %d.\n", PORTA);
        chiudiDatabase();
        return 1;
    }

    chiudiDatabase();
    return 0;
}

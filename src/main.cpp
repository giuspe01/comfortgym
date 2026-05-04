// Palestra digitale — entry point del backend
// Caso di studio ITPS, traccia 2.

#include <iostream>
#include <string>

#include "httplib.h"

namespace {
    constexpr const char* INDIRIZZO = "0.0.0.0";
    constexpr int PORTA = 8080;
}

int main() {
    httplib::Server server;

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

    std::cout << "Palestra digitale - server in ascolto su http://localhost:"
              << PORTA << std::endl;
    std::cout << "Premi Ctrl+C per terminare." << std::endl;

    if (!server.listen(INDIRIZZO, PORTA)) {
        std::cerr << "Errore: impossibile avviare il server sulla porta "
                  << PORTA << "." << std::endl;
        return 1;
    }
    return 0;
}

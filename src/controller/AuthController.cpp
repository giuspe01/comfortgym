#include "AuthController.h"

#include <cstring>

#include "modello/Utente.h"
#include "servizi/Auth.h"
#include "utils/HttpUtils.h"

// Legge il valore del cookie 'sessione' in 'tokenOut'.
// Ritorna 1 se trovato, 0 se assente.
static int leggiCookieSessione(const httplib::Request& req, char* tokenOut) {
    if (!req.has_header("Cookie")) {
        return 0;
    }
    std::string cookies = req.get_header_value("Cookie");

    const std::string chiave = "sessione=";
    size_t pos = cookies.find(chiave);
    if (pos == std::string::npos) {
        return 0;
    }
    pos += chiave.length();
    size_t fine = cookies.find(';', pos);
    if (fine == std::string::npos) {
        fine = cookies.length();
    }

    std::string valore = cookies.substr(pos, fine - pos);
    if ((int)valore.length() > LUNGHEZZA_TOKEN) {
        valore = valore.substr(0, LUNGHEZZA_TOKEN);
    }
    strncpy(tokenOut, valore.c_str(), LUNGHEZZA_TOKEN);
    tokenOut[LUNGHEZZA_TOKEN] = '\0';
    return 1;
}

int idUtenteCorrente(const httplib::Request& req) {
    char token[LUNGHEZZA_TOKEN + 1];
    if (!leggiCookieSessione(req, token)) {
        return 0;
    }
    return verificaSessione(token);
}

void registraRotteAuth(httplib::Server& server) {

    // ----- POST /api/auth/register -----
    server.Post("/api/auth/register",
        [](const httplib::Request& req, httplib::Response& res) {
            nlohmann::json corpo;
            if (!leggiCorpoJson(req, res, corpo)) {
                return;
            }

            char msgErrore[200] = {0};
            int idNuovo = registra(corpo, msgErrore, sizeof(msgErrore));
            if (idNuovo == 0) {
                rispondiErrore(res, 400, msgErrore);
                return;
            }

            nlohmann::json risposta;
            risposta["id"] = idNuovo;
            risposta["messaggio"] = "Registrazione completata. Ora puoi accedere.";
            rispondiOk(res, risposta);
        });

    // ----- POST /api/auth/login -----
    server.Post("/api/auth/login",
        [](const httplib::Request& req, httplib::Response& res) {
            nlohmann::json corpo;
            if (!leggiCorpoJson(req, res, corpo)) {
                return;
            }

            std::string username = corpo.value("username", "");
            std::string password = corpo.value("password", "");

            int idUtente = login(username.c_str(), password.c_str());
            if (idUtente == 0) {
                rispondiErrore(res, 401, "Nome utente o password errati");
                return;
            }

            // Sessione + cookie httpOnly.
            char token[LUNGHEZZA_TOKEN + 1];
            creaSessione(idUtente, token);

            char setCookie[256];
            snprintf(setCookie, sizeof(setCookie),
                     "sessione=%s; HttpOnly; Path=/; SameSite=Lax; Max-Age=%d",
                     token, DURATA_SESSIONE_SEC);
            res.set_header("Set-Cookie", setCookie);

            Utente* u = caricaUtente(idUtente);
            if (u == NULL) {
                rispondiErrore(res, 500, "Errore caricamento utente");
                return;
            }

            nlohmann::json risposta;
            risposta["utente"] = u->toJson();
            rispondiOk(res, risposta);

            delete u;
        });

    // ----- POST /api/auth/logout -----
    server.Post("/api/auth/logout",
        [](const httplib::Request& req, httplib::Response& res) {
            char token[LUNGHEZZA_TOKEN + 1];
            if (leggiCookieSessione(req, token)) {
                rimuoviSessione(token);
            }

            // Max-Age=0: il browser cancella il cookie.
            res.set_header("Set-Cookie",
                           "sessione=; HttpOnly; Path=/; SameSite=Lax; Max-Age=0");

            nlohmann::json risposta;
            risposta["messaggio"] = "Logout effettuato";
            rispondiOk(res, risposta);
        });

    // ----- GET /api/me -----
    server.Get("/api/me",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente = idUtenteCorrente(req);
            if (idUtente == 0) {
                rispondiErrore(res, 401, "Non autenticato");
                return;
            }

            Utente* u = caricaUtente(idUtente);
            if (u == NULL) {
                rispondiErrore(res, 500, "Errore caricamento utente");
                return;
            }

            nlohmann::json risposta;
            risposta["utente"] = u->toJson();
            rispondiOk(res, risposta);

            delete u;
        });
}

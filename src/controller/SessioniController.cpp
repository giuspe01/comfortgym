#include "SessioniController.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "controller/AuthController.h"
#include "modello/Sessione.h"
#include "modello/Utente.h"
#include "servizi/Auth.h"
#include "servizi/ServizioSessioni.h"
#include "utils/HttpUtils.h"

static int estraiIdDaPath(const std::string& path) {
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos) return 0;
    return atoi(path.c_str() + pos + 1);
}

static int siaCliente(const httplib::Request& req, httplib::Response& res,
                      int& idUtenteOut) {
    idUtenteOut = idUtenteCorrente(req);
    if (idUtenteOut == 0) {
        rispondiErrore(res, 401, "Non autenticato");
        return 0;
    }
    Utente* u = caricaUtente(idUtenteOut);
    if (u == NULL) {
        rispondiErrore(res, 500, "Errore caricamento utente");
        return 0;
    }
    int ok = (strcmp(u->tipo(), "cliente") == 0);
    delete u;
    if (!ok) {
        rispondiErrore(res, 403, "Solo i clienti possono fare questa operazione");
        return 0;
    }
    return 1;
}

static int siaProfessionista(const httplib::Request& req, httplib::Response& res,
                             int& idUtenteOut) {
    idUtenteOut = idUtenteCorrente(req);
    if (idUtenteOut == 0) {
        rispondiErrore(res, 401, "Non autenticato");
        return 0;
    }
    Utente* u = caricaUtente(idUtenteOut);
    if (u == NULL) {
        rispondiErrore(res, 500, "Errore caricamento utente");
        return 0;
    }
    int ok = (strcmp(u->tipo(), "professionista") == 0);
    delete u;
    if (!ok) {
        rispondiErrore(res, 403, "Solo i professionisti possono fare questa operazione");
        return 0;
    }
    return 1;
}

void registraRotteSessioni(httplib::Server& server) {

    // ----- POST /api/sessioni (cliente registra una sessione) -----
    server.Post("/api/sessioni",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaCliente(req, res, idUtente)) return;

            nlohmann::json corpo;
            if (!leggiCorpoJson(req, res, corpo)) return;

            char msgErr[200] = {0};
            int idNuovo = registraSessione(idUtente, corpo, msgErr, sizeof(msgErr));
            if (idNuovo == 0) {
                rispondiErrore(res, 400, msgErr);
                return;
            }
            nlohmann::json r;
            r["id"] = idNuovo;
            r["messaggio"] = "Sessione registrata";
            rispondiOk(res, r);
        });

    // ----- GET /api/sessioni/me (storico del cliente loggato) -----
    server.Get("/api/sessioni/me",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaCliente(req, res, idUtente)) return;

            Sessione elenco[MAX_SESSIONI_PER_QUERY];
            int n = listaSessioniCliente(idUtente, elenco, MAX_SESSIONI_PER_QUERY);

            nlohmann::json arr = nlohmann::json::array();
            for (int i = 0; i < n; i++) arr.push_back(elenco[i].toJson());

            nlohmann::json r;
            r["sessioni"] = arr;
            rispondiOk(res, r);
        });

    // ----- DELETE /api/sessioni/<id> (cliente cancella una propria sessione) -----
    server.Delete(R"(/api/sessioni/\d+)",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaCliente(req, res, idUtente)) return;

            int id = estraiIdDaPath(req.path);
            if (!cancellaSessione(id, idUtente)) {
                rispondiErrore(res, 404, "Sessione non trovata");
                return;
            }
            nlohmann::json r;
            r["messaggio"] = "Sessione eliminata";
            rispondiOk(res, r);
        });

    // ----- GET /api/professionista/attivita -----
    // Statistiche aggregate per ogni programma creato dal professionista.
    server.Get("/api/professionista/attivita",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaProfessionista(req, res, idUtente)) return;

            nlohmann::json arr;
            calcolaStatistichePerProfessionista(idUtente, arr);

            nlohmann::json r;
            r["attivita"] = arr;
            rispondiOk(res, r);
        });

    // ----- GET /api/professionista/sessioni -----
    // Singole sessioni svolte sui programmi di questo professionista,
    // con nome e dati anagrafici del cliente.
    server.Get("/api/professionista/sessioni",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaProfessionista(req, res, idUtente)) return;

            nlohmann::json arr;
            listaSessioniPerProfessionista(idUtente, arr);

            nlohmann::json r;
            r["sessioni"] = arr;
            rispondiOk(res, r);
        });
}

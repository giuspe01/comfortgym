#include "ProgrammiController.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "controller/AuthController.h"
#include "modello/Programma.h"
#include "modello/Utente.h"
#include "servizi/Auth.h"
#include "servizi/ServizioProgrammi.h"
#include "utils/HttpUtils.h"

// Estrae l'id intero dal percorso "/api/programmi/<n>" o simili.
// Cerca l'ultimo '/' e converte cio' che segue.
static int estraiIdDaPath(const std::string& path) {
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos) return 0;
    return atoi(path.c_str() + pos + 1);
}

// Restituisce 1 se l'utente loggato e' un professionista.
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

// Restituisce 1 se l'utente loggato e' un cliente.
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

void registraRotteProgrammi(httplib::Server& server) {

    // ----- GET /api/programmi (catalogo con filtri) -----
    server.Get("/api/programmi",
        [](const httplib::Request& req, httplib::Response& res) {
            if (idUtenteCorrente(req) == 0) {
                rispondiErrore(res, 401, "Non autenticato");
                return;
            }

            const char* obiettivo = req.has_param("obiettivo")
                ? req.get_param_value("obiettivo").c_str() : NULL;
            const char* difficolta = req.has_param("difficolta")
                ? req.get_param_value("difficolta").c_str() : NULL;
            int durataMax = req.has_param("durataMax")
                ? atoi(req.get_param_value("durataMax").c_str()) : 0;

            Programma elenco[MAX_PROGRAMMI_PER_QUERY];
            int n = listaProgrammi(elenco, MAX_PROGRAMMI_PER_QUERY,
                                   obiettivo, difficolta, durataMax);

            nlohmann::json arr = nlohmann::json::array();
            for (int i = 0; i < n; i++) {
                arr.push_back(elenco[i].toJson());
            }
            nlohmann::json risposta;
            risposta["programmi"] = arr;
            rispondiOk(res, risposta);
        });

    // ----- GET /api/programmi/<id> (dettaglio con esercizi) -----
    server.Get(R"(/api/programmi/\d+)",
        [](const httplib::Request& req, httplib::Response& res) {
            if (idUtenteCorrente(req) == 0) {
                rispondiErrore(res, 401, "Non autenticato");
                return;
            }
            int id = estraiIdDaPath(req.path);
            Programma* p = caricaProgramma(id);
            if (p == NULL) {
                rispondiErrore(res, 404, "Programma non trovato");
                return;
            }

            Esercizio esercizi[MAX_ESERCIZI_PER_PROGRAMMA];
            int n = caricaEsercizi(id, esercizi, MAX_ESERCIZI_PER_PROGRAMMA);

            nlohmann::json arr = nlohmann::json::array();
            for (int i = 0; i < n; i++) {
                arr.push_back(esercizi[i].toJson());
            }

            nlohmann::json risposta;
            risposta["programma"] = p->toJson();
            risposta["esercizi"] = arr;
            rispondiOk(res, risposta);

            delete p;
        });

    // ----- POST /api/programmi (crea, solo professionista) -----
    server.Post("/api/programmi",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaProfessionista(req, res, idUtente)) return;

            nlohmann::json corpo;
            if (!leggiCorpoJson(req, res, corpo)) return;

            char msgErr[200] = {0};
            int idNuovo = creaProgramma(corpo, idUtente, msgErr, sizeof(msgErr));
            if (idNuovo == 0) {
                rispondiErrore(res, 400, msgErr);
                return;
            }
            nlohmann::json r;
            r["id"] = idNuovo;
            r["messaggio"] = "Programma creato";
            rispondiOk(res, r);
        });

    // ----- PUT /api/programmi/<id> (modifica, solo creatore) -----
    server.Put(R"(/api/programmi/\d+)",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaProfessionista(req, res, idUtente)) return;

            nlohmann::json corpo;
            if (!leggiCorpoJson(req, res, corpo)) return;

            int id = estraiIdDaPath(req.path);
            char msgErr[200] = {0};
            if (!aggiornaProgramma(id, corpo, idUtente, msgErr, sizeof(msgErr))) {
                rispondiErrore(res, 400, msgErr);
                return;
            }
            nlohmann::json r;
            r["messaggio"] = "Programma aggiornato";
            rispondiOk(res, r);
        });

    // ----- DELETE /api/programmi/<id> (elimina, solo creatore) -----
    server.Delete(R"(/api/programmi/\d+)",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaProfessionista(req, res, idUtente)) return;

            int id = estraiIdDaPath(req.path);
            if (!cancellaProgramma(id, idUtente)) {
                rispondiErrore(res, 404, "Programma non trovato o non sei il creatore");
                return;
            }
            nlohmann::json r;
            r["messaggio"] = "Programma eliminato";
            rispondiOk(res, r);
        });

    // ----- GET /api/professionista/programmi -----
    server.Get("/api/professionista/programmi",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaProfessionista(req, res, idUtente)) return;

            Programma elenco[MAX_PROGRAMMI_PER_QUERY];
            int n = listaProgrammiDelCreatore(idUtente, elenco, MAX_PROGRAMMI_PER_QUERY);

            nlohmann::json arr = nlohmann::json::array();
            for (int i = 0; i < n; i++) {
                arr.push_back(elenco[i].toJson());
            }
            nlohmann::json risposta;
            risposta["programmi"] = arr;
            rispondiOk(res, risposta);
        });

    // ----- POST /api/cliente/programma (seleziona programma) -----
    server.Post("/api/cliente/programma",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaCliente(req, res, idUtente)) return;

            nlohmann::json corpo;
            if (!leggiCorpoJson(req, res, corpo)) return;

            int idProgramma = 0;
            if (corpo.contains("idProgramma") && corpo["idProgramma"].is_number_integer()) {
                idProgramma = corpo["idProgramma"].get<int>();
            }

            if (!selezionaProgrammaCliente(idUtente, idProgramma)) {
                rispondiErrore(res, 400, "Selezione non valida");
                return;
            }
            nlohmann::json r;
            r["idProgrammaCorrente"] = idProgramma;
            r["messaggio"] = (idProgramma == 0)
                ? "Selezione rimossa"
                : "Programma selezionato";
            rispondiOk(res, r);
        });
}

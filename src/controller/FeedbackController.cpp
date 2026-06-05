#include "FeedbackController.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "controller/AuthController.h"
#include "modello/Feedback.h"
#include "modello/Utente.h"
#include "servizi/Auth.h"
#include "servizi/ServizioFeedback.h"
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

void registraRotteFeedback(httplib::Server& server) {

    // ----- POST /api/feedback (cliente invia / aggiorna) -----
    server.Post("/api/feedback",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaCliente(req, res, idUtente)) return;

            nlohmann::json corpo;
            if (!leggiCorpoJson(req, res, corpo)) return;

            char msgErr[200] = {0};
            int idFb = inviaFeedback(idUtente, corpo, msgErr, sizeof(msgErr));
            if (idFb == 0) {
                rispondiErrore(res, 400, msgErr);
                return;
            }
            nlohmann::json r;
            r["id"] = idFb;
            r["messaggio"] = "Feedback salvato";
            rispondiOk(res, r);
        });

    // ----- GET /api/feedback/programma/<id> (cliente: il suo feedback per quel programma) -----
    server.Get(R"(/api/feedback/programma/\d+)",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaCliente(req, res, idUtente)) return;

            int idProgramma = estraiIdDaPath(req.path);
            Feedback* f = caricaFeedbackCliente(idUtente, idProgramma);

            nlohmann::json r;
            if (f == NULL) {
                r["feedback"] = nullptr;
            } else {
                r["feedback"] = f->toJson();
                delete f;
            }
            rispondiOk(res, r);
        });

    // ----- GET /api/professionista/feedback (tutti i feedback ricevuti) -----
    server.Get("/api/professionista/feedback",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaProfessionista(req, res, idUtente)) return;

            nlohmann::json arr;
            listaFeedbackProfessionista(idUtente, arr);

            nlohmann::json r;
            r["feedback"] = arr;
            rispondiOk(res, r);
        });
}

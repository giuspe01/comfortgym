#include "PianiController.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "controller/AuthController.h"
#include "modello/PianoNutrizionale.h"
#include "modello/Utente.h"
#include "servizi/Auth.h"
#include "servizi/ServizioPiani.h"
#include "utils/HttpUtils.h"

static int estraiIdDaPath(const std::string& path) {
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos) return 0;
    return atoi(path.c_str() + pos + 1);
}

// Verifica che l'utente loggato sia un professionista. Imposta idUtenteOut.
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

void registraRottePiani(httplib::Server& server) {

    // ----- GET /api/piani (catalogo con filtro calorie) -----
    server.Get("/api/piani",
        [](const httplib::Request& req, httplib::Response& res) {
            if (idUtenteCorrente(req) == 0) {
                rispondiErrore(res, 401, "Non autenticato");
                return;
            }
            int calorieMax = req.has_param("calorieMax")
                ? atoi(req.get_param_value("calorieMax").c_str()) : 0;

            PianoNutrizionale elenco[MAX_PIANI_PER_QUERY];
            int n = listaPiani(elenco, MAX_PIANI_PER_QUERY, calorieMax);

            nlohmann::json arr = nlohmann::json::array();
            for (int i = 0; i < n; i++) arr.push_back(elenco[i].toJson());
            nlohmann::json r;
            r["piani"] = arr;
            rispondiOk(res, r);
        });

    // ----- GET /api/piani/<id> (dettaglio + pasti) -----
    server.Get(R"(/api/piani/\d+)",
        [](const httplib::Request& req, httplib::Response& res) {
            if (idUtenteCorrente(req) == 0) {
                rispondiErrore(res, 401, "Non autenticato");
                return;
            }
            int id = estraiIdDaPath(req.path);
            PianoNutrizionale* p = caricaPiano(id);
            if (p == NULL) {
                rispondiErrore(res, 404, "Piano non trovato");
                return;
            }

            Pasto pasti[MAX_PASTI_PER_PIANO];
            int n = caricaPasti(id, pasti, MAX_PASTI_PER_PIANO);

            nlohmann::json arr = nlohmann::json::array();
            for (int i = 0; i < n; i++) arr.push_back(pasti[i].toJson());

            nlohmann::json r;
            r["piano"] = p->toJson();
            r["pasti"] = arr;
            rispondiOk(res, r);

            delete p;
        });

    // ----- POST /api/piani (crea, solo professionista) -----
    server.Post("/api/piani",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaProfessionista(req, res, idUtente)) return;

            nlohmann::json corpo;
            if (!leggiCorpoJson(req, res, corpo)) return;

            char msgErr[200] = {0};
            int idNuovo = creaPiano(corpo, idUtente, msgErr, sizeof(msgErr));
            if (idNuovo == 0) {
                rispondiErrore(res, 400, msgErr);
                return;
            }
            nlohmann::json r;
            r["id"] = idNuovo;
            r["messaggio"] = "Piano creato";
            rispondiOk(res, r);
        });

    // ----- PUT /api/piani/<id> -----
    server.Put(R"(/api/piani/\d+)",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaProfessionista(req, res, idUtente)) return;

            nlohmann::json corpo;
            if (!leggiCorpoJson(req, res, corpo)) return;

            int id = estraiIdDaPath(req.path);
            char msgErr[200] = {0};
            if (!aggiornaPiano(id, corpo, idUtente, msgErr, sizeof(msgErr))) {
                rispondiErrore(res, 400, msgErr);
                return;
            }
            nlohmann::json r;
            r["messaggio"] = "Piano aggiornato";
            rispondiOk(res, r);
        });

    // ----- DELETE /api/piani/<id> -----
    server.Delete(R"(/api/piani/\d+)",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaProfessionista(req, res, idUtente)) return;

            int id = estraiIdDaPath(req.path);
            if (!cancellaPiano(id, idUtente)) {
                rispondiErrore(res, 404, "Piano non trovato o non sei il creatore");
                return;
            }
            nlohmann::json r;
            r["messaggio"] = "Piano eliminato";
            rispondiOk(res, r);
        });

    // ----- GET /api/professionista/piani -----
    server.Get("/api/professionista/piani",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaProfessionista(req, res, idUtente)) return;

            PianoNutrizionale elenco[MAX_PIANI_PER_QUERY];
            int n = listaPianiDelCreatore(idUtente, elenco, MAX_PIANI_PER_QUERY);

            nlohmann::json arr = nlohmann::json::array();
            for (int i = 0; i < n; i++) arr.push_back(elenco[i].toJson());
            nlohmann::json r;
            r["piani"] = arr;
            rispondiOk(res, r);
        });

    // ----- POST /api/cliente/piano (seleziona piano) -----
    server.Post("/api/cliente/piano",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaCliente(req, res, idUtente)) return;

            nlohmann::json corpo;
            if (!leggiCorpoJson(req, res, corpo)) return;

            int idPiano = 0;
            if (corpo.contains("idPiano") && corpo["idPiano"].is_number_integer()) {
                idPiano = corpo["idPiano"].get<int>();
            }

            if (!selezionaPianoCliente(idUtente, idPiano)) {
                rispondiErrore(res, 400, "Selezione non valida");
                return;
            }
            nlohmann::json r;
            r["idPianoCorrente"] = idPiano;
            r["messaggio"] = (idPiano == 0) ? "Selezione rimossa" : "Piano selezionato";
            rispondiOk(res, r);
        });
}

#include "AderenzaController.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "controller/AuthController.h"
#include "modello/Aderenza.h"
#include "modello/Utente.h"
#include "servizi/Auth.h"
#include "servizi/ServizioAderenza.h"
#include "utils/HttpUtils.h"

static int estraiIdDaPath(const std::string& path) {
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos) return 0;
    return atoi(path.c_str() + pos + 1);
}

// Per path tipo "/api/professionista/aderenza/5/consiglio" estrae "5".
static int estraiIdPenultimo(const std::string& path) {
    size_t last = path.find_last_of('/');
    if (last == std::string::npos || last == 0) return 0;
    size_t prev = path.find_last_of('/', last - 1);
    if (prev == std::string::npos) return 0;
    return atoi(path.c_str() + prev + 1);
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

void registraRotteAderenza(httplib::Server& server) {

    // POST /api/aderenza - registra o aggiorna l'aderenza di oggi
    server.Post("/api/aderenza",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaCliente(req, res, idUtente)) return;

            nlohmann::json corpo;
            if (!leggiCorpoJson(req, res, corpo)) return;

            char msgErr[200] = {0};
            int idNuovo = registraAderenza(idUtente, corpo, msgErr, sizeof(msgErr));
            if (idNuovo == 0) {
                rispondiErrore(res, 400, msgErr);
                return;
            }
            nlohmann::json r;
            r["id"] = idNuovo;
            r["messaggio"] = "Aderenza registrata";
            rispondiOk(res, r);
        });

    // GET /api/aderenza/me - lista delle aderenze del cliente
    server.Get("/api/aderenza/me",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaCliente(req, res, idUtente)) return;

            Aderenza elenco[MAX_ADERENZE_PER_QUERY];
            int n = listaAderenzeCliente(idUtente, elenco, MAX_ADERENZE_PER_QUERY);

            nlohmann::json arr = nlohmann::json::array();
            for (int i = 0; i < n; i++) arr.push_back(elenco[i].toJson());

            nlohmann::json r;
            r["aderenze"] = arr;
            rispondiOk(res, r);
        });

    // DELETE /api/aderenza/<id> - cancellazione
    server.Delete(R"(/api/aderenza/\d+)",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente;
            if (!siaCliente(req, res, idUtente)) return;

            int id = estraiIdDaPath(req.path);
            if (!cancellaAderenza(id, idUtente)) {
                rispondiErrore(res, 404, "Aderenza non trovata");
                return;
            }
            nlohmann::json r;
            r["messaggio"] = "Aderenza cancellata";
            rispondiOk(res, r);
        });

    // PUT /api/professionista/aderenza/<id>/consiglio - nutrizionista aggiunge il consiglio
    server.Put(R"(/api/professionista/aderenza/\d+/consiglio)",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente = idUtenteCorrente(req);
            if (idUtente == 0) { rispondiErrore(res, 401, "Non autenticato"); return; }

            Utente* u = caricaUtente(idUtente);
            if (u == NULL) { rispondiErrore(res, 500, "Errore caricamento utente"); return; }
            if (strcmp(u->tipo(), "professionista") != 0) {
                delete u;
                rispondiErrore(res, 403, "Solo i professionisti possono aggiungere consigli");
                return;
            }
            Professionista* p = (Professionista*)u;
            int ok = (strcmp(p->specializzazione, "nutrizionista") == 0 ||
                      strcmp(p->specializzazione, "entrambi") == 0);
            delete u;
            if (!ok) {
                rispondiErrore(res, 403, "Solo i nutrizionisti possono commentare le aderenze");
                return;
            }

            nlohmann::json corpo;
            if (!leggiCorpoJson(req, res, corpo)) return;

            std::string consiglio = "";
            if (corpo.contains("consiglio") && corpo["consiglio"].is_string()) {
                consiglio = corpo["consiglio"].get<std::string>();
            }

            int idAderenza = estraiIdPenultimo(req.path);
            if (!aggiornaConsiglioPro(idAderenza, consiglio.c_str(), idUtente)) {
                rispondiErrore(res, 404, "Aderenza non trovata o non appartiene a un tuo piano");
                return;
            }
            nlohmann::json r;
            r["messaggio"] = "Consiglio aggiornato";
            rispondiOk(res, r);
        });
}

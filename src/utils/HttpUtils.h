// HttpUtils.h - Helper per costruire risposte HTTP in formato JSON.
//
// Tutte le rotte /api/... del nostro server rispondono in JSON: invece di
// duplicare ogni volta res.status = ... e res.set_content(...), usiamo
// queste funzioni che fanno la stessa cosa con un nome chiaro.

#ifndef PALESTRA_HTTPUTILS_H
#define PALESTRA_HTTPUTILS_H

#include "httplib.h"
#include "json.hpp"

// Risposta di successo (HTTP 200). I 'dati' vengono serializzati in JSON.
void rispondiOk(httplib::Response& res, const nlohmann::json& dati);

// Risposta di errore con il codice HTTP indicato (es. 400 = richiesta non valida,
// 401 = non autenticato, 404 = non trovato, 409 = conflitto, 500 = errore server).
// Il corpo della risposta e' un JSON della forma { "errore": "..." }.
void rispondiErrore(httplib::Response& res, int statusHttp, const char* messaggio);

// Prova a interpretare il corpo della richiesta come JSON.
// Ritorna 1 se il parse e' andato a buon fine (e scrive il risultato in 'destinazione'),
// 0 se il corpo non era un JSON valido (e in tal caso scrive da solo
// una risposta di errore 400 in 'res', cosi' il chiamante puo' fare 'return').
int leggiCorpoJson(const httplib::Request& req,
                   httplib::Response& res,
                   nlohmann::json& destinazione);

#endif

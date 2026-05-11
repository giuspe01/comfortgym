// HttpUtils.h - Helper per richieste HTTP.
//
// Tutte le rotte rispondono in JSON.

#ifndef PALESTRA_HTTPUTILS_H
#define PALESTRA_HTTPUTILS_H

#include "httplib.h"
#include "json.hpp"

// Risposta di successo (HTTP 200). I 'dati' vengono serializzati in JSON.
void rispondiOk(httplib::Response& res, const nlohmann::json& dati);

// Risposta di errore con il codice HTTP indicato (400 = richiesta non valida,
// 401 = non autenticato, 404 = non trovato, 409 = conflitto, 500 = errore server).
void rispondiErrore(httplib::Response& res, int statusHttp, const char* messaggio);

// Interpretazione corpo della richiesta come JSON.
// Ritorna 1 se il parse e' andato a buon fine (e scrive il risultato in 'destinazione'),
// 0 se il corpo non era un JSON valido (scrive una risposta di errore 400 in 'res').
int leggiCorpoJson(const httplib::Request& req,
                   httplib::Response& res,
                   nlohmann::json& destinazione);

#endif

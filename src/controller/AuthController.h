// AuthController.h - Rotte HTTP /api/auth/... e /api/me.

#ifndef PALESTRA_AUTHCONTROLLER_H
#define PALESTRA_AUTHCONTROLLER_H

#include "httplib.h"

// Estrae il cookie di sessione e ritorna l'id dell'utente, 0 se non loggato.
int idUtenteCorrente(const httplib::Request& req);

// Aggancia le rotte di autenticazione al server.
void registraRotteAuth(httplib::Server& server);

#endif

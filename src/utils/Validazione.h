// Validazione.h - Controlli sui dati.
//
// La validazione avviene sempre lato server, anche
// se l'HTML lo controlla gia': un utente malintenzionato potrebbe bypassare
// i controlli del frontend.

#ifndef PALESTRA_VALIDAZIONE_H
#define PALESTRA_VALIDAZIONE_H

// Limiti generali.
#define LUNGHEZZA_MIN_USERNAME  3
#define LUNGHEZZA_MAX_USERNAME 20
#define LUNGHEZZA_MIN_PASSWORD  6
#define LUNGHEZZA_MAX_PASSWORD 50
#define LUNGHEZZA_MIN_EMAIL     5
#define LUNGHEZZA_MAX_EMAIL   100
#define LUNGHEZZA_MAX_NOME     50

// Controlla che 'str' non sia NULL e che la sua lunghezza sia compresa
// tra 'lunghezzaMin' e 'lunghezzaMax' (estremi inclusi).
int validaStringa(const char* str, int lunghezzaMin, int lunghezzaMax);

// Username: solo lettere, numeri, '_', e tra 3 e 20 caratteri.
int validaUsername(const char* username);

// Password: tra 6 e 50 caratteri.
int validaPassword(const char* password);

// Email: deve contenere almeno una '@' e un '.', e stare nei limiti di lunghezza.
int validaEmail(const char* email);

// Nome o cognome: max 50 caratteri.
int validaNome(const char* nome);

#endif

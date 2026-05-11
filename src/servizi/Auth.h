// Auth.h - Registrazione, login, gestione delle sessioni web.

#ifndef PALESTRA_AUTH_H
#define PALESTRA_AUTH_H

#include "json.hpp"
#include "modello/Utente.h"

#define LUNGHEZZA_TOKEN 32
#define DURATA_SESSIONE_SEC (60 * 60 * 24 * 7)   // 7 giorni

// Registra un nuovo utente. Ritorna l'id assegnato, 0 in caso di errore.
// In caso di errore scrive la causa in 'msgErrore'.
int registra(const nlohmann::json& dati, char* msgErrore, int dimErr);

// Verifica le credenziali. Ritorna l'id dell'utente, 0 se non valide.
int login(const char* username, const char* password);

// Carica un utente dal DB come Cliente* o Professionista* tramite Utente*.
// Il chiamante deve fare delete dopo l'uso. NULL se non esiste.
Utente* caricaUtente(int idUtente);

// Genera un token, lo salva in sessioni_web, lo scrive in 'tokenOut'.
// Il buffer deve essere lungo almeno LUNGHEZZA_TOKEN + 1.
void creaSessione(int idUtente, char* tokenOut);

// Ritorna l'id utente associato al token, 0 se non valido o scaduto.
int verificaSessione(const char* token);

// Cancella la sessione (logout).
void rimuoviSessione(const char* token);

#endif

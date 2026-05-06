// Database.h - Wrapper per il database SQLite.
//
// Il programma usa un singolo file SQLite (data/palestra.db) per persistere
// tutti i dati. Il puntatore globale 'g_db' rappresenta la connessione aperta:
// viene inizializzato a NULL e popolato dalla apriDatabase() chiamata nel main.
//
// Stile scelto: variabile globale + funzioni libere, come fa SOCRASYS con le
// sue variabili globali. E' la soluzione piu' semplice da spiegare, anche se
// in un progetto piu' grande passeremmo la connessione come parametro.

#ifndef PALESTRA_DATABASE_H
#define PALESTRA_DATABASE_H

#include "sqlite3.h"

// Connessione globale al database. NULL finche' non chiamiamo apriDatabase.
extern sqlite3* g_db;

// Apre (o crea) il file di database al percorso indicato e prepara lo schema:
// esegue PRAGMA foreign_keys = ON e CREATE TABLE IF NOT EXISTS per tutte le
// tabelle del progetto. Ritorna 1 se tutto va bene, 0 in caso di errore.
int apriDatabase(const char* percorso);

// Chiude il database (se aperto). Sicuro da chiamare anche se g_db e' NULL.
void chiudiDatabase();

// Esegue una query SQL che non restituisce righe (CREATE, INSERT, UPDATE,
// DELETE). In caso di errore, stampa il messaggio su stderr.
// Ritorna 1 se ok, 0 altrimenti.
int eseguiSql(const char* sql);

#endif

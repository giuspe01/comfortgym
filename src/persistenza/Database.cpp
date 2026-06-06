#include "Database.h"

#include <cstdio>

sqlite3* g_db = NULL;

// Schema completo del database. Tutte le istruzioni sono "IF NOT EXISTS" cosi'
// l'inizializzazione e' idempotente: si puo' richiamare senza errori.
//
// PRAGMA foreign_keys = ON va eseguito ad ogni apertura della connessione,
// perche' SQLite parte di default con i vincoli di chiave esterna disattivati.
//
// Convenzioni adottate:
//   - id INTEGER PRIMARY KEY AUTOINCREMENT in tutte le tabelle (chiave naturale)
//   - testo libero come TEXT (SQLite non distingue VARCHAR(n))
//   - le date sono salvate come TEXT in formato ISO 8601 ("2026-05-06")
//   - i booleani come INTEGER 0 o 1 (SQLite non ha tipo bool nativo)
//   - i campi specifici di Cliente o Professionista sono nullable nella stessa
//     tabella 'utenti', perche' un utente e' o cliente o professionista, mai
//     entrambi: discriminiamo col campo 'tipo'.
static const char* SCHEMA_SQL =
    "PRAGMA foreign_keys = ON;"

    "CREATE TABLE IF NOT EXISTS utenti ("
    "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "    username TEXT NOT NULL UNIQUE,"
    "    password_hash TEXT NOT NULL,"
    "    nome TEXT NOT NULL,"
    "    cognome TEXT NOT NULL,"
    "    email TEXT NOT NULL,"
    "    data_registrazione TEXT NOT NULL,"
    "    tipo TEXT NOT NULL,"                      // 'cliente' | 'professionista'
    "    eta INTEGER,"                             // campi cliente
    "    peso REAL,"
    "    altezza REAL,"
    "    obiettivo TEXT,"                          // 'perdita_peso'|'massa'|'mantenimento'
    "    id_programma_corrente INTEGER,"
    "    id_piano_corrente INTEGER,"
    "    id_professionista_assegnato INTEGER,"
    "    specializzazione TEXT"                    // campo professionista: 'trainer'|'nutrizionista'|'entrambi'
    ");"

    "CREATE TABLE IF NOT EXISTS programmi ("
    "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "    nome TEXT NOT NULL,"
    "    obiettivo TEXT NOT NULL,"
    "    difficolta TEXT NOT NULL,"
    "    durata_settimane INTEGER NOT NULL,"
    "    id_creatore INTEGER NOT NULL REFERENCES utenti(id),"
    "    immagine TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS esercizi ("
    "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "    id_programma INTEGER NOT NULL REFERENCES programmi(id) ON DELETE CASCADE,"
    "    nome TEXT NOT NULL,"
    "    serie INTEGER NOT NULL,"
    "    ripetizioni INTEGER NOT NULL,"
    "    riposo_sec INTEGER NOT NULL,"
    "    note TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS piani ("
    "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "    nome TEXT NOT NULL,"
    "    calorie_giornaliere INTEGER NOT NULL,"
    "    id_creatore INTEGER NOT NULL REFERENCES utenti(id),"
    "    consigli_giornalieri TEXT,"                      // suggerimenti del professionista
    "    immagine TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS aderenza_nutrizionale ("
    "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "    id_cliente INTEGER NOT NULL REFERENCES utenti(id) ON DELETE CASCADE,"
    "    id_piano INTEGER NOT NULL REFERENCES piani(id) ON DELETE CASCADE,"
    "    data TEXT NOT NULL,"                              // ISO "AAAA-MM-GG"
    "    percentuale INTEGER NOT NULL,"                    // 0..100
    "    note TEXT,"
    "    UNIQUE(id_cliente, data)"                         // un'unica registrazione per giorno
    ");"

    "CREATE TABLE IF NOT EXISTS pasti ("
    "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "    id_piano INTEGER NOT NULL REFERENCES piani(id) ON DELETE CASCADE,"
    "    tipo TEXT NOT NULL,"                      // 'colazione' | 'pranzo' | 'cena' | 'spuntino'
    "    descrizione TEXT NOT NULL,"
    "    calorie INTEGER NOT NULL"
    ");"

    "CREATE TABLE IF NOT EXISTS sessioni ("
    "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "    id_cliente INTEGER NOT NULL REFERENCES utenti(id),"
    "    id_programma INTEGER NOT NULL REFERENCES programmi(id),"
    "    data TEXT NOT NULL,"
    "    durata_min INTEGER NOT NULL,"
    "    completata INTEGER NOT NULL,"             // 0 o 1
    "    note TEXT"
    ");"

    "CREATE TABLE IF NOT EXISTS feedback ("
    "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "    id_cliente INTEGER NOT NULL REFERENCES utenti(id),"
    "    id_programma INTEGER NOT NULL REFERENCES programmi(id),"
    "    voto INTEGER NOT NULL,"                   // 1..5
    "    commento TEXT,"
    "    data TEXT NOT NULL"
    ");"

    "CREATE TABLE IF NOT EXISTS feedback_piani ("
    "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "    id_cliente INTEGER NOT NULL REFERENCES utenti(id),"
    "    id_piano INTEGER NOT NULL REFERENCES piani(id),"
    "    voto INTEGER NOT NULL,"                   // 1..5
    "    commento TEXT,"
    "    data TEXT NOT NULL"
    ");"

    "CREATE TABLE IF NOT EXISTS sessioni_web ("
    "    token TEXT PRIMARY KEY,"
    "    id_utente INTEGER NOT NULL REFERENCES utenti(id) ON DELETE CASCADE,"
    "    scadenza TEXT NOT NULL"
    ");";

int apriDatabase(const char* percorso) {
    int rc = sqlite3_open(percorso, &g_db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Errore apertura database '%s': %s\n",
                percorso, sqlite3_errmsg(g_db));
        sqlite3_close(g_db);
        g_db = NULL;
        return 0;
    }

    // Crea le tabelle se non ci sono.
    char* errMsg = NULL;
    rc = sqlite3_exec(g_db, SCHEMA_SQL, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Errore creazione schema: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(g_db);
        g_db = NULL;
        return 0;
    }

    // Aggiunge colonne introdotte in versioni successive.
    // ALTER TABLE fallisce silenziosamente se la colonna esiste gia'.
    sqlite3_exec(g_db,
        "ALTER TABLE aderenza_nutrizionale ADD COLUMN consiglio_pro TEXT;",
        NULL, NULL, NULL);

    return 1;
}

void chiudiDatabase() {
    if (g_db != NULL) {
        sqlite3_close(g_db);
        g_db = NULL;
    }
}

int eseguiSql(const char* sql) {
    if (g_db == NULL) {
        fprintf(stderr, "Errore: database non aperto.\n");
        return 0;
    }
    char* errMsg = NULL;
    int rc = sqlite3_exec(g_db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Errore SQL: %s\n", errMsg);
        sqlite3_free(errMsg);
        return 0;
    }
    return 1;
}

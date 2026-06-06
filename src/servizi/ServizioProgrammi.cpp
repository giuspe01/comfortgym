#include "ServizioProgrammi.h"

#include <cstdio>
#include <cstring>

#include "persistenza/Database.h"

// ----- Helper interni -----

// Legge una stringa da un campo JSON in 'buffer'. Se manca scrive '\0'.
static void leggiStringa(const nlohmann::json& j, const char* chiave,
                         char* buffer, int dimBuffer) {
    if (j.contains(chiave) && j[chiave].is_string()) {
        std::string s = j[chiave].get<std::string>();
        strncpy(buffer, s.c_str(), dimBuffer - 1);
        buffer[dimBuffer - 1] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

// Legge un intero da un campo JSON. Se manca ritorna 0.
static int leggiIntero(const nlohmann::json& j, const char* chiave) {
    if (j.contains(chiave) && j[chiave].is_number_integer()) {
        return j[chiave].get<int>();
    }
    return 0;
}

// Copia una colonna di testo da SQLite in 'dest'.
static void copiaColonnaTesto(sqlite3_stmt* stmt, int iCol, char* dest, int dimDest) {
    const unsigned char* val = sqlite3_column_text(stmt, iCol);
    if (val == NULL) {
        dest[0] = '\0';
        return;
    }
    strncpy(dest, (const char*)val, dimDest - 1);
    dest[dimDest - 1] = '\0';
}

// Controlli base sul JSON del programma. Ritorna 1 se valido, 0 altrimenti.
static int validaDatiProgramma(const nlohmann::json& dati,
                               char* msgErr, int dimErr) {
    char nome[LUNG_NOME_PROGRAMMA];
    char obiettivo[LUNG_OBIETTIVO];
    char difficolta[LUNG_DIFFICOLTA];
    leggiStringa(dati, "nome", nome, sizeof(nome));
    leggiStringa(dati, "obiettivo", obiettivo, sizeof(obiettivo));
    leggiStringa(dati, "difficolta", difficolta, sizeof(difficolta));
    int durata = leggiIntero(dati, "durataSettimane");

    if (strlen(nome) < 2) {
        snprintf(msgErr, dimErr, "Nome programma troppo corto");
        return 0;
    }
    if (strlen(obiettivo) < 1) {
        snprintf(msgErr, dimErr, "Obiettivo mancante");
        return 0;
    }
    if (strcmp(difficolta, "principiante") != 0
        && strcmp(difficolta, "intermedio") != 0
        && strcmp(difficolta, "avanzato") != 0) {
        snprintf(msgErr, dimErr, "Difficolta' non valida");
        return 0;
    }
    if (durata < 1 || durata > 52) {
        snprintf(msgErr, dimErr, "Durata in settimane non valida (1-52)");
        return 0;
    }
    return 1;
}

// Inserisce gli esercizi dell'array JSON 'esercizi' per il programma indicato.
// Ritorna 1 se ok, 0 altrimenti.
static int inserisciEsercizi(int idProgramma, const nlohmann::json& esercizi) {
    if (!esercizi.is_array()) {
        return 1;   // nessun esercizio: il programma puo' essere creato vuoto
    }

    const char* sql =
        "INSERT INTO esercizi (id_programma, nome, serie, ripetizioni, riposo_sec, note) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    for (const auto& e : esercizi) {
        char nome[LUNG_NOME_ESERCIZIO];
        char note[LUNG_NOTE];
        leggiStringa(e, "nome", nome, sizeof(nome));
        leggiStringa(e, "note", note, sizeof(note));
        int serie = leggiIntero(e, "serie");
        int rip = leggiIntero(e, "ripetizioni");
        int rip_riposo = leggiIntero(e, "riposoSec");

        if (strlen(nome) < 1) continue;   // saltiamo gli esercizi senza nome

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
            return 0;
        }
        sqlite3_bind_int(stmt, 1, idProgramma);
        sqlite3_bind_text(stmt, 2, nome, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, serie);
        sqlite3_bind_int(stmt, 4, rip);
        sqlite3_bind_int(stmt, 5, rip_riposo);
        sqlite3_bind_text(stmt, 6, note, -1, SQLITE_STATIC);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE) {
            return 0;
        }
    }
    return 1;
}

// ----- creaProgramma -----

int creaProgramma(const nlohmann::json& dati, int idCreatore,
                  char* msgErr, int dimErr) {
    if (!validaDatiProgramma(dati, msgErr, dimErr)) {
        return 0;
    }

    char nome[LUNG_NOME_PROGRAMMA];
    char obiettivo[LUNG_OBIETTIVO];
    char difficolta[LUNG_DIFFICOLTA];
    char immagine[LUNG_IMMAGINE_PROG];
    leggiStringa(dati, "nome", nome, sizeof(nome));
    leggiStringa(dati, "obiettivo", obiettivo, sizeof(obiettivo));
    leggiStringa(dati, "difficolta", difficolta, sizeof(difficolta));
    leggiStringa(dati, "immagine", immagine, sizeof(immagine));
    int durata = leggiIntero(dati, "durataSettimane");

    // Transazione: programma + esercizi devono andare insieme.
    sqlite3_exec(g_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    const char* sql =
        "INSERT INTO programmi (nome, obiettivo, difficolta, durata_settimane, id_creatore, immagine) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        snprintf(msgErr, dimErr, "Errore interno (prepare)");
        return 0;
    }
    sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, obiettivo, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, difficolta, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, durata);
    sqlite3_bind_int(stmt, 5, idCreatore);
    sqlite3_bind_text(stmt, 6, immagine, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        snprintf(msgErr, dimErr, "Errore creazione programma");
        return 0;
    }

    int idProgramma = (int)sqlite3_last_insert_rowid(g_db);

    if (dati.contains("esercizi")) {
        if (!inserisciEsercizi(idProgramma, dati["esercizi"])) {
            sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
            snprintf(msgErr, dimErr, "Errore inserimento esercizi");
            return 0;
        }
    }

    sqlite3_exec(g_db, "COMMIT;", NULL, NULL, NULL);
    return idProgramma;
}

// ----- aggiornaProgramma -----

int aggiornaProgramma(int idProgramma, const nlohmann::json& dati,
                      int idCreatore, char* msgErr, int dimErr) {
    if (!validaDatiProgramma(dati, msgErr, dimErr)) {
        return 0;
    }

    // Verifica che chi richiede l'update sia il creatore.
    const char* sqlCheck = "SELECT id_creatore FROM programmi WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sqlCheck, -1, &stmt, NULL) != SQLITE_OK) {
        snprintf(msgErr, dimErr, "Errore interno");
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idProgramma);
    int proprietario = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        proprietario = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (proprietario == 0) {
        snprintf(msgErr, dimErr, "Programma non trovato");
        return 0;
    }
    if (proprietario != idCreatore) {
        snprintf(msgErr, dimErr, "Non sei il creatore di questo programma");
        return 0;
    }

    char nome[LUNG_NOME_PROGRAMMA];
    char obiettivo[LUNG_OBIETTIVO];
    char difficolta[LUNG_DIFFICOLTA];
    char immagine[LUNG_IMMAGINE_PROG];
    leggiStringa(dati, "nome", nome, sizeof(nome));
    leggiStringa(dati, "obiettivo", obiettivo, sizeof(obiettivo));
    leggiStringa(dati, "difficolta", difficolta, sizeof(difficolta));
    leggiStringa(dati, "immagine", immagine, sizeof(immagine));
    int durata = leggiIntero(dati, "durataSettimane");

    sqlite3_exec(g_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    // UPDATE del programma.
    const char* sqlUpd =
        "UPDATE programmi SET nome = ?, obiettivo = ?, difficolta = ?, "
        "                     durata_settimane = ?, immagine = ? WHERE id = ?;";
    if (sqlite3_prepare_v2(g_db, sqlUpd, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        snprintf(msgErr, dimErr, "Errore interno (prepare update)");
        return 0;
    }
    sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, obiettivo, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, difficolta, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, durata);
    sqlite3_bind_text(stmt, 5, immagine, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, idProgramma);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        snprintf(msgErr, dimErr, "Errore update programma");
        return 0;
    }

    // Sostituiamo tutti gli esercizi: cancelliamo e reinseriamo.
    const char* sqlDel = "DELETE FROM esercizi WHERE id_programma = ?;";
    if (sqlite3_prepare_v2(g_db, sqlDel, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        snprintf(msgErr, dimErr, "Errore interno (prepare delete esercizi)");
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idProgramma);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (dati.contains("esercizi")) {
        if (!inserisciEsercizi(idProgramma, dati["esercizi"])) {
            sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
            snprintf(msgErr, dimErr, "Errore inserimento esercizi");
            return 0;
        }
    }

    sqlite3_exec(g_db, "COMMIT;", NULL, NULL, NULL);
    return 1;
}

// ----- cancellaProgramma -----

int cancellaProgramma(int idProgramma, int idCreatore) {
    // Verifica ownership prima di toccare il DB.
    const char* sqlCheck = "SELECT id_creatore FROM programmi WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sqlCheck, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idProgramma);
    int proprietario = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        proprietario = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    if (proprietario != idCreatore) {
        return 0;   // non e' tuo o non esiste
    }

    // Transazione: ripulisce tutti i riferimenti, poi cancella.
    // (sessioni e feedback hanno FK verso programmi senza CASCADE, vanno
    // ripuliti prima; gli esercizi cascadano.)
    sqlite3_exec(g_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    const char* cleanup[] = {
        "DELETE FROM feedback WHERE id_programma = ?;",
        "DELETE FROM sessioni WHERE id_programma = ?;",
        "UPDATE utenti SET id_programma_corrente = 0 WHERE id_programma_corrente = ?;",
        "DELETE FROM programmi WHERE id = ?;"
    };
    for (int i = 0; i < 4; i++) {
        if (sqlite3_prepare_v2(g_db, cleanup[i], -1, &stmt, NULL) != SQLITE_OK) {
            sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
            return 0;
        }
        sqlite3_bind_int(stmt, 1, idProgramma);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
            return 0;
        }
    }

    sqlite3_exec(g_db, "COMMIT;", NULL, NULL, NULL);
    return 1;
}

// ----- caricaProgramma -----

Programma* caricaProgramma(int idProgramma) {
    const char* sql =
        "SELECT id, nome, obiettivo, difficolta, durata_settimane, id_creatore, immagine "
        "FROM programmi WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    sqlite3_bind_int(stmt, 1, idProgramma);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return NULL;
    }

    Programma* p = new Programma();
    p->id = sqlite3_column_int(stmt, 0);
    copiaColonnaTesto(stmt, 1, p->nome, sizeof(p->nome));
    copiaColonnaTesto(stmt, 2, p->obiettivo, sizeof(p->obiettivo));
    copiaColonnaTesto(stmt, 3, p->difficolta, sizeof(p->difficolta));
    p->durataSettimane = sqlite3_column_int(stmt, 4);
    p->idCreatore = sqlite3_column_int(stmt, 5);
    copiaColonnaTesto(stmt, 6, p->immagine, sizeof(p->immagine));

    sqlite3_finalize(stmt);
    return p;
}

// ----- caricaEsercizi -----

int caricaEsercizi(int idProgramma, Esercizio* dest, int maxNum) {
    const char* sql =
        "SELECT id, id_programma, nome, serie, ripetizioni, riposo_sec, note "
        "FROM esercizi WHERE id_programma = ? ORDER BY id;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idProgramma);

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < maxNum) {
        Esercizio& e = dest[n];
        e.id = sqlite3_column_int(stmt, 0);
        e.idProgramma = sqlite3_column_int(stmt, 1);
        copiaColonnaTesto(stmt, 2, e.nome, sizeof(e.nome));
        e.serie = sqlite3_column_int(stmt, 3);
        e.ripetizioni = sqlite3_column_int(stmt, 4);
        e.riposoSec = sqlite3_column_int(stmt, 5);
        copiaColonnaTesto(stmt, 6, e.note, sizeof(e.note));
        n++;
    }
    sqlite3_finalize(stmt);
    return n;
}

// ----- listaProgrammi -----

int listaProgrammi(Programma* dest, int maxNum,
                   const char* obiettivo,
                   const char* difficolta,
                   int durataMax) {
    // Costruiamo la query dinamicamente in base ai filtri presenti.
    char sql[700];
    strcpy(sql, "SELECT id, nome, obiettivo, difficolta, durata_settimane, id_creatore, immagine "
                "FROM programmi WHERE 1=1");
    if (obiettivo != NULL && obiettivo[0] != '\0') {
        strcat(sql, " AND obiettivo = ?");
    }
    if (difficolta != NULL && difficolta[0] != '\0') {
        strcat(sql, " AND difficolta = ?");
    }
    if (durataMax > 0) {
        strcat(sql, " AND durata_settimane <= ?");
    }
    strcat(sql, " ORDER BY id DESC;");

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    int idx = 1;
    if (obiettivo != NULL && obiettivo[0] != '\0') {
        sqlite3_bind_text(stmt, idx++, obiettivo, -1, SQLITE_STATIC);
    }
    if (difficolta != NULL && difficolta[0] != '\0') {
        sqlite3_bind_text(stmt, idx++, difficolta, -1, SQLITE_STATIC);
    }
    if (durataMax > 0) {
        sqlite3_bind_int(stmt, idx++, durataMax);
    }

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < maxNum) {
        Programma& p = dest[n];
        p.id = sqlite3_column_int(stmt, 0);
        copiaColonnaTesto(stmt, 1, p.nome, sizeof(p.nome));
        copiaColonnaTesto(stmt, 2, p.obiettivo, sizeof(p.obiettivo));
        copiaColonnaTesto(stmt, 3, p.difficolta, sizeof(p.difficolta));
        p.durataSettimane = sqlite3_column_int(stmt, 4);
        p.idCreatore = sqlite3_column_int(stmt, 5);
        copiaColonnaTesto(stmt, 6, p.immagine, sizeof(p.immagine));
        n++;
    }
    sqlite3_finalize(stmt);
    return n;
}

// ----- listaProgrammiDelCreatore -----

int listaProgrammiDelCreatore(int idCreatore, Programma* dest, int maxNum) {
    const char* sql =
        "SELECT id, nome, obiettivo, difficolta, durata_settimane, id_creatore, immagine "
        "FROM programmi WHERE id_creatore = ? ORDER BY id DESC;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idCreatore);

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < maxNum) {
        Programma& p = dest[n];
        p.id = sqlite3_column_int(stmt, 0);
        copiaColonnaTesto(stmt, 1, p.nome, sizeof(p.nome));
        copiaColonnaTesto(stmt, 2, p.obiettivo, sizeof(p.obiettivo));
        copiaColonnaTesto(stmt, 3, p.difficolta, sizeof(p.difficolta));
        p.durataSettimane = sqlite3_column_int(stmt, 4);
        p.idCreatore = sqlite3_column_int(stmt, 5);
        copiaColonnaTesto(stmt, 6, p.immagine, sizeof(p.immagine));
        n++;
    }
    sqlite3_finalize(stmt);
    return n;
}

// ----- selezionaProgrammaCliente -----

int selezionaProgrammaCliente(int idCliente, int idProgramma) {
    // Se l'id e' diverso da 0, verifichiamo che il programma esista.
    if (idProgramma != 0) {
        const char* sqlCheck = "SELECT id FROM programmi WHERE id = ?;";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(g_db, sqlCheck, -1, &stmt, NULL) != SQLITE_OK) {
            return 0;
        }
        sqlite3_bind_int(stmt, 1, idProgramma);
        int trovato = (sqlite3_step(stmt) == SQLITE_ROW);
        sqlite3_finalize(stmt);
        if (!trovato) {
            return 0;
        }
    }

    const char* sql = "UPDATE utenti SET id_programma_corrente = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idProgramma);
    sqlite3_bind_int(stmt, 2, idCliente);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? 1 : 0;
}

#include "ServizioPiani.h"

#include <cstdio>
#include <cstring>

#include "persistenza/Database.h"

// ----- Helper interni -----

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

static int leggiIntero(const nlohmann::json& j, const char* chiave) {
    if (j.contains(chiave) && j[chiave].is_number_integer()) {
        return j[chiave].get<int>();
    }
    return 0;
}

static void copiaColonnaTesto(sqlite3_stmt* stmt, int iCol, char* dest, int dimDest) {
    const unsigned char* val = sqlite3_column_text(stmt, iCol);
    if (val == NULL) {
        dest[0] = '\0';
        return;
    }
    strncpy(dest, (const char*)val, dimDest - 1);
    dest[dimDest - 1] = '\0';
}

static int validaDatiPiano(const nlohmann::json& dati, char* msgErr, int dimErr) {
    char nome[LUNG_NOME_PIANO];
    leggiStringa(dati, "nome", nome, sizeof(nome));
    int calorie = leggiIntero(dati, "calorieGiornaliere");

    if (strlen(nome) < 2) {
        snprintf(msgErr, dimErr, "Nome del piano troppo corto");
        return 0;
    }
    if (calorie < 500 || calorie > 6000) {
        snprintf(msgErr, dimErr, "Calorie giornaliere fuori range (500-6000)");
        return 0;
    }
    return 1;
}

// Inserisce i pasti del JSON array per il piano indicato.
static int inserisciPasti(int idPiano, const nlohmann::json& pasti) {
    if (!pasti.is_array()) {
        return 1;
    }

    const char* sql =
        "INSERT INTO pasti (id_piano, tipo, descrizione, calorie) VALUES (?, ?, ?, ?);";

    for (const auto& p : pasti) {
        char tipo[LUNG_TIPO_PASTO];
        char descr[LUNG_DESCRIZIONE];
        leggiStringa(p, "tipo", tipo, sizeof(tipo));
        leggiStringa(p, "descrizione", descr, sizeof(descr));
        int cal = leggiIntero(p, "calorie");

        // Valida il tipo del pasto.
        if (strcmp(tipo, "colazione") != 0
            && strcmp(tipo, "pranzo") != 0
            && strcmp(tipo, "cena") != 0
            && strcmp(tipo, "spuntino") != 0) {
            continue;   // salta pasti senza un tipo valido
        }
        if (strlen(descr) < 1) {
            continue;
        }

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
            return 0;
        }
        sqlite3_bind_int(stmt, 1, idPiano);
        sqlite3_bind_text(stmt, 2, tipo, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, descr, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, cal);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE) return 0;
    }
    return 1;
}

// ----- creaPiano -----

int creaPiano(const nlohmann::json& dati, int idCreatore,
              char* msgErr, int dimErr) {
    if (!validaDatiPiano(dati, msgErr, dimErr)) {
        return 0;
    }

    char nome[LUNG_NOME_PIANO];
    leggiStringa(dati, "nome", nome, sizeof(nome));
    int calorie = leggiIntero(dati, "calorieGiornaliere");

    sqlite3_exec(g_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    const char* sql =
        "INSERT INTO piani (nome, calorie_giornaliere, id_creatore) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        snprintf(msgErr, dimErr, "Errore interno (prepare)");
        return 0;
    }
    sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, calorie);
    sqlite3_bind_int(stmt, 3, idCreatore);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        snprintf(msgErr, dimErr, "Errore creazione piano");
        return 0;
    }

    int idPiano = (int)sqlite3_last_insert_rowid(g_db);

    if (dati.contains("pasti")) {
        if (!inserisciPasti(idPiano, dati["pasti"])) {
            sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
            snprintf(msgErr, dimErr, "Errore inserimento pasti");
            return 0;
        }
    }

    sqlite3_exec(g_db, "COMMIT;", NULL, NULL, NULL);
    return idPiano;
}

// ----- aggiornaPiano -----

int aggiornaPiano(int idPiano, const nlohmann::json& dati,
                  int idCreatore, char* msgErr, int dimErr) {
    if (!validaDatiPiano(dati, msgErr, dimErr)) {
        return 0;
    }

    // Verifica ownership.
    const char* sqlCheck = "SELECT id_creatore FROM piani WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sqlCheck, -1, &stmt, NULL) != SQLITE_OK) {
        snprintf(msgErr, dimErr, "Errore interno");
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idPiano);
    int proprietario = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        proprietario = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (proprietario == 0) {
        snprintf(msgErr, dimErr, "Piano non trovato");
        return 0;
    }
    if (proprietario != idCreatore) {
        snprintf(msgErr, dimErr, "Non sei il creatore di questo piano");
        return 0;
    }

    char nome[LUNG_NOME_PIANO];
    leggiStringa(dati, "nome", nome, sizeof(nome));
    int calorie = leggiIntero(dati, "calorieGiornaliere");

    sqlite3_exec(g_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    const char* sqlUpd =
        "UPDATE piani SET nome = ?, calorie_giornaliere = ? WHERE id = ?;";
    if (sqlite3_prepare_v2(g_db, sqlUpd, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        snprintf(msgErr, dimErr, "Errore interno (prepare update)");
        return 0;
    }
    sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, calorie);
    sqlite3_bind_int(stmt, 3, idPiano);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        snprintf(msgErr, dimErr, "Errore update piano");
        return 0;
    }

    // Sostituiamo tutti i pasti.
    const char* sqlDel = "DELETE FROM pasti WHERE id_piano = ?;";
    if (sqlite3_prepare_v2(g_db, sqlDel, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
        snprintf(msgErr, dimErr, "Errore interno (prepare delete pasti)");
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idPiano);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (dati.contains("pasti")) {
        if (!inserisciPasti(idPiano, dati["pasti"])) {
            sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
            snprintf(msgErr, dimErr, "Errore inserimento pasti");
            return 0;
        }
    }

    sqlite3_exec(g_db, "COMMIT;", NULL, NULL, NULL);
    return 1;
}

// ----- cancellaPiano -----

int cancellaPiano(int idPiano, int idCreatore) {
    const char* sql = "DELETE FROM piani WHERE id = ? AND id_creatore = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idPiano);
    sqlite3_bind_int(stmt, 2, idCreatore);
    int rc = sqlite3_step(stmt);
    int righeCancellate = sqlite3_changes(g_db);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE && righeCancellate > 0) ? 1 : 0;
}

// ----- caricaPiano -----

PianoNutrizionale* caricaPiano(int idPiano) {
    const char* sql =
        "SELECT id, nome, calorie_giornaliere, id_creatore FROM piani WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    sqlite3_bind_int(stmt, 1, idPiano);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return NULL;
    }

    PianoNutrizionale* p = new PianoNutrizionale();
    p->id = sqlite3_column_int(stmt, 0);
    copiaColonnaTesto(stmt, 1, p->nome, sizeof(p->nome));
    p->calorieGiornaliere = sqlite3_column_int(stmt, 2);
    p->idCreatore = sqlite3_column_int(stmt, 3);

    sqlite3_finalize(stmt);
    return p;
}

// ----- caricaPasti -----

int caricaPasti(int idPiano, Pasto* dest, int maxNum) {
    const char* sql =
        "SELECT id, id_piano, tipo, descrizione, calorie FROM pasti "
        "WHERE id_piano = ? ORDER BY id;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idPiano);

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < maxNum) {
        Pasto& p = dest[n];
        p.id = sqlite3_column_int(stmt, 0);
        p.idPiano = sqlite3_column_int(stmt, 1);
        copiaColonnaTesto(stmt, 2, p.tipo, sizeof(p.tipo));
        copiaColonnaTesto(stmt, 3, p.descrizione, sizeof(p.descrizione));
        p.calorie = sqlite3_column_int(stmt, 4);
        n++;
    }
    sqlite3_finalize(stmt);
    return n;
}

// ----- listaPiani -----

int listaPiani(PianoNutrizionale* dest, int maxNum, int calorieMax) {
    char sql[400];
    strcpy(sql, "SELECT id, nome, calorie_giornaliere, id_creatore FROM piani WHERE 1=1");
    if (calorieMax > 0) {
        strcat(sql, " AND calorie_giornaliere <= ?");
    }
    strcat(sql, " ORDER BY id DESC;");

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    if (calorieMax > 0) {
        sqlite3_bind_int(stmt, 1, calorieMax);
    }

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < maxNum) {
        PianoNutrizionale& p = dest[n];
        p.id = sqlite3_column_int(stmt, 0);
        copiaColonnaTesto(stmt, 1, p.nome, sizeof(p.nome));
        p.calorieGiornaliere = sqlite3_column_int(stmt, 2);
        p.idCreatore = sqlite3_column_int(stmt, 3);
        n++;
    }
    sqlite3_finalize(stmt);
    return n;
}

// ----- listaPianiDelCreatore -----

int listaPianiDelCreatore(int idCreatore, PianoNutrizionale* dest, int maxNum) {
    const char* sql =
        "SELECT id, nome, calorie_giornaliere, id_creatore FROM piani "
        "WHERE id_creatore = ? ORDER BY id DESC;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idCreatore);

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < maxNum) {
        PianoNutrizionale& p = dest[n];
        p.id = sqlite3_column_int(stmt, 0);
        copiaColonnaTesto(stmt, 1, p.nome, sizeof(p.nome));
        p.calorieGiornaliere = sqlite3_column_int(stmt, 2);
        p.idCreatore = sqlite3_column_int(stmt, 3);
        n++;
    }
    sqlite3_finalize(stmt);
    return n;
}

// ----- selezionaPianoCliente -----

int selezionaPianoCliente(int idCliente, int idPiano) {
    if (idPiano != 0) {
        const char* sqlCheck = "SELECT id FROM piani WHERE id = ?;";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(g_db, sqlCheck, -1, &stmt, NULL) != SQLITE_OK) {
            return 0;
        }
        sqlite3_bind_int(stmt, 1, idPiano);
        int trovato = (sqlite3_step(stmt) == SQLITE_ROW);
        sqlite3_finalize(stmt);
        if (!trovato) return 0;
    }

    const char* sql = "UPDATE utenti SET id_piano_corrente = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idPiano);
    sqlite3_bind_int(stmt, 2, idCliente);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? 1 : 0;
}

#include "ServizioAderenza.h"

#include <cstdio>
#include <cstring>
#include <ctime>

#include "persistenza/Database.h"

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
    if (val == NULL) { dest[0] = '\0'; return; }
    strncpy(dest, (const char*)val, dimDest - 1);
    dest[dimDest - 1] = '\0';
}

static void dataOggi(char* buffer) {
    time_t t = time(NULL);
    struct tm* info = localtime(&t);
    strftime(buffer, 11, "%Y-%m-%d", info);
}

// Trova un'aderenza esistente per (cliente, data). Ritorna l'id o 0.
static int idAderenzaEsistente(int idCliente, const char* data) {
    const char* sql =
        "SELECT id FROM aderenza_nutrizionale WHERE id_cliente = ? AND data = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, idCliente);
    sqlite3_bind_text(stmt, 2, data, -1, SQLITE_STATIC);
    int id = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        id = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return id;
}

// ----- registraAderenza -----

int registraAderenza(int idCliente, const nlohmann::json& dati,
                     char* msgErr, int dimErr) {
    int idPiano = leggiIntero(dati, "idPiano");
    int percentuale = leggiIntero(dati, "percentuale");
    char data[LUNG_DATA_ADERENZA];
    char note[LUNG_NOTE_ADERENZA];
    leggiStringa(dati, "data", data, sizeof(data));
    leggiStringa(dati, "note", note, sizeof(note));

    if (data[0] == '\0') {
        dataOggi(data);
    }

    if (idPiano <= 0) {
        snprintf(msgErr, dimErr, "idPiano mancante");
        return 0;
    }
    if (percentuale < 0 || percentuale > 100) {
        snprintf(msgErr, dimErr, "Percentuale fuori range (0-100)");
        return 0;
    }

    // Verifica esistenza piano.
    const char* sqlCheck = "SELECT id FROM piani WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sqlCheck, -1, &stmt, NULL) != SQLITE_OK) {
        snprintf(msgErr, dimErr, "Errore interno");
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idPiano);
    int trovato = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    if (!trovato) {
        snprintf(msgErr, dimErr, "Piano inesistente");
        return 0;
    }

    int esistente = idAderenzaEsistente(idCliente, data);

    if (esistente > 0) {
        const char* sql =
            "UPDATE aderenza_nutrizionale SET id_piano = ?, percentuale = ?, note = ? "
            "WHERE id = ?;";
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
            snprintf(msgErr, dimErr, "Errore interno (prepare update)");
            return 0;
        }
        sqlite3_bind_int(stmt, 1, idPiano);
        sqlite3_bind_int(stmt, 2, percentuale);
        sqlite3_bind_text(stmt, 3, note, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, esistente);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE) {
            snprintf(msgErr, dimErr, "Errore aggiornamento aderenza");
            return 0;
        }
        return esistente;
    }

    const char* sql =
        "INSERT INTO aderenza_nutrizionale (id_cliente, id_piano, data, percentuale, note) "
        "VALUES (?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        snprintf(msgErr, dimErr, "Errore interno (prepare)");
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idCliente);
    sqlite3_bind_int(stmt, 2, idPiano);
    sqlite3_bind_text(stmt, 3, data, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, percentuale);
    sqlite3_bind_text(stmt, 5, note, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        snprintf(msgErr, dimErr, "Errore registrazione aderenza");
        return 0;
    }
    return (int)sqlite3_last_insert_rowid(g_db);
}

// ----- cancellaAderenza -----

int cancellaAderenza(int idAderenza, int idCliente) {
    const char* sql =
        "DELETE FROM aderenza_nutrizionale WHERE id = ? AND id_cliente = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, idAderenza);
    sqlite3_bind_int(stmt, 2, idCliente);
    int rc = sqlite3_step(stmt);
    int righe = sqlite3_changes(g_db);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE && righe > 0) ? 1 : 0;
}

// ----- listaAderenzeCliente -----

int listaAderenzeCliente(int idCliente, Aderenza* dest, int maxNum) {
    const char* sql =
        "SELECT a.id, a.id_cliente, a.id_piano, "
        "       COALESCE(p.nome, '') AS nome_piano, "
        "       a.data, a.percentuale, a.note, "
        "       COALESCE(a.consiglio_pro, '') AS consiglio_pro "
        "FROM aderenza_nutrizionale a "
        "LEFT JOIN piani p ON p.id = a.id_piano "
        "WHERE a.id_cliente = ? "
        "ORDER BY a.data DESC, a.id DESC;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, idCliente);

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < maxNum) {
        Aderenza& a = dest[n];
        a.id = sqlite3_column_int(stmt, 0);
        a.idCliente = sqlite3_column_int(stmt, 1);
        a.idPiano = sqlite3_column_int(stmt, 2);
        copiaColonnaTesto(stmt, 3, a.nomePiano, sizeof(a.nomePiano));
        copiaColonnaTesto(stmt, 4, a.data, sizeof(a.data));
        a.percentuale = sqlite3_column_int(stmt, 5);
        copiaColonnaTesto(stmt, 6, a.note, sizeof(a.note));
        copiaColonnaTesto(stmt, 7, a.consiglioPro, sizeof(a.consiglioPro));
        n++;
    }
    sqlite3_finalize(stmt);
    return n;
}

// ----- aggiornaConsiglioPro -----

int aggiornaConsiglioPro(int idAderenza, const char* consiglio, int idNutrizionista) {
    // Aggiorna solo se l'aderenza e' su un piano creato da questo nutrizionista.
    const char* sql =
        "UPDATE aderenza_nutrizionale SET consiglio_pro = ? "
        "WHERE id = ? "
        "  AND id_piano IN (SELECT id FROM piani WHERE id_creatore = ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, consiglio, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, idAderenza);
    sqlite3_bind_int(stmt, 3, idNutrizionista);
    int rc = sqlite3_step(stmt);
    int righe = sqlite3_changes(g_db);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE && righe > 0) ? 1 : 0;
}

// ----- listaAderenzePerNutrizionista -----

void listaAderenzePerNutrizionista(int idNutrizionista, nlohmann::json& destinazione) {
    destinazione = nlohmann::json::array();

    const char* sql =
        "SELECT a.id, a.data, a.percentuale, a.note, "
        "       COALESCE(a.consiglio_pro, '') AS consiglio_pro, "
        "       p.id AS id_piano, p.nome AS nome_piano, "
        "       u.id AS id_cliente, u.nome, u.cognome, u.email, "
        "       u.eta, u.peso, u.altezza, u.obiettivo "
        "FROM aderenza_nutrizionale a "
        "JOIN piani p ON p.id = a.id_piano "
        "JOIN utenti u ON u.id = a.id_cliente "
        "WHERE p.id_creatore = ? "
        "ORDER BY a.data DESC, a.id DESC;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return;
    sqlite3_bind_int(stmt, 1, idNutrizionista);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        nlohmann::json riga;
        riga["id"] = sqlite3_column_int(stmt, 0);

        char buf[400];
        copiaColonnaTesto(stmt, 1, buf, sizeof(buf));
        riga["data"] = buf;

        riga["percentuale"] = sqlite3_column_int(stmt, 2);

        copiaColonnaTesto(stmt, 3, buf, sizeof(buf));
        riga["note"] = buf;

        copiaColonnaTesto(stmt, 4, buf, sizeof(buf));
        riga["consiglioPro"] = buf;

        riga["idPiano"] = sqlite3_column_int(stmt, 5);
        copiaColonnaTesto(stmt, 6, buf, sizeof(buf));
        riga["nomePiano"] = buf;

        riga["idCliente"] = sqlite3_column_int(stmt, 7);

        char nome[64], cognome[64];
        copiaColonnaTesto(stmt, 8, nome, sizeof(nome));
        copiaColonnaTesto(stmt, 9, cognome, sizeof(cognome));
        char nomeCompleto[140];
        snprintf(nomeCompleto, sizeof(nomeCompleto), "%s %s", nome, cognome);
        riga["nomeCliente"] = nomeCompleto;

        copiaColonnaTesto(stmt, 10, buf, sizeof(buf));
        riga["emailCliente"] = buf;

        riga["etaCliente"] = sqlite3_column_int(stmt, 11);
        riga["pesoCliente"] = sqlite3_column_double(stmt, 12);
        riga["altezzaCliente"] = sqlite3_column_double(stmt, 13);

        copiaColonnaTesto(stmt, 14, buf, sizeof(buf));
        riga["obiettivoCliente"] = buf;

        destinazione.push_back(riga);
    }
    sqlite3_finalize(stmt);
}

// ----- attivitaAgregatePiani -----

void attivitaAgregatePiani(int idNutrizionista, nlohmann::json& destinazione) {
    destinazione = nlohmann::json::array();

    const char* sql =
        "SELECT p.id, p.nome, "
        "       COUNT(a.id) AS totale, "
        "       ROUND(COALESCE(AVG(a.percentuale), 0)) AS media_perc, "
        "       MAX(a.data) AS ultima_data "
        "FROM piani p "
        "LEFT JOIN aderenza_nutrizionale a ON a.id_piano = p.id "
        "WHERE p.id_creatore = ? "
        "GROUP BY p.id, p.nome "
        "ORDER BY p.id DESC;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return;
    sqlite3_bind_int(stmt, 1, idNutrizionista);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        nlohmann::json riga;
        riga["idPiano"] = sqlite3_column_int(stmt, 0);

        char buf[200];
        copiaColonnaTesto(stmt, 1, buf, sizeof(buf));
        riga["nomePiano"] = buf;

        riga["totale"] = sqlite3_column_int(stmt, 2);
        riga["mediaPercentuale"] = sqlite3_column_int(stmt, 3);

        if (sqlite3_column_type(stmt, 4) == SQLITE_NULL) {
            riga["ultimaData"] = nlohmann::json(nullptr);
        } else {
            copiaColonnaTesto(stmt, 4, buf, sizeof(buf));
            riga["ultimaData"] = buf;
        }

        destinazione.push_back(riga);
    }
    sqlite3_finalize(stmt);
}

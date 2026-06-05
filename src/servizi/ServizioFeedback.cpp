#include "ServizioFeedback.h"

#include <cstdio>
#include <cstring>
#include <ctime>

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
    if (val == NULL) { dest[0] = '\0'; return; }
    strncpy(dest, (const char*)val, dimDest - 1);
    dest[dimDest - 1] = '\0';
}

static void dataOggi(char* buffer) {
    time_t t = time(NULL);
    struct tm* info = localtime(&t);
    strftime(buffer, 11, "%Y-%m-%d", info);
}

// Cerca un feedback esistente del cliente sul programma. Ritorna id, 0 se assente.
static int idFeedbackEsistente(int idCliente, int idProgramma) {
    const char* sql =
        "SELECT id FROM feedback WHERE id_cliente = ? AND id_programma = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, idCliente);
    sqlite3_bind_int(stmt, 2, idProgramma);
    int id = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        id = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return id;
}

// ----- inviaFeedback -----

int inviaFeedback(int idCliente, const nlohmann::json& dati,
                  char* msgErr, int dimErr) {
    int idProgramma = leggiIntero(dati, "idProgramma");
    int voto = leggiIntero(dati, "voto");
    char commento[LUNG_COMMENTO_FB];
    leggiStringa(dati, "commento", commento, sizeof(commento));

    if (idProgramma <= 0) {
        snprintf(msgErr, dimErr, "idProgramma mancante");
        return 0;
    }
    if (voto < 1 || voto > 5) {
        snprintf(msgErr, dimErr, "Voto fuori range (1-5)");
        return 0;
    }

    // Verifica esistenza programma.
    const char* sqlCheck = "SELECT id FROM programmi WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sqlCheck, -1, &stmt, NULL) != SQLITE_OK) {
        snprintf(msgErr, dimErr, "Errore interno");
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idProgramma);
    int trovato = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    if (!trovato) {
        snprintf(msgErr, dimErr, "Programma inesistente");
        return 0;
    }

    char dataOggiStr[11];
    dataOggi(dataOggiStr);

    int esistente = idFeedbackEsistente(idCliente, idProgramma);

    if (esistente > 0) {
        // Aggiorno il feedback esistente.
        const char* sql =
            "UPDATE feedback SET voto = ?, commento = ?, data = ? WHERE id = ?;";
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
            snprintf(msgErr, dimErr, "Errore interno (prepare update)");
            return 0;
        }
        sqlite3_bind_int(stmt, 1, voto);
        sqlite3_bind_text(stmt, 2, commento, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, dataOggiStr, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, esistente);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE) {
            snprintf(msgErr, dimErr, "Errore aggiornamento feedback");
            return 0;
        }
        return esistente;
    }

    // INSERT nuovo feedback.
    const char* sql =
        "INSERT INTO feedback (id_cliente, id_programma, voto, commento, data) "
        "VALUES (?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        snprintf(msgErr, dimErr, "Errore interno (prepare insert)");
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idCliente);
    sqlite3_bind_int(stmt, 2, idProgramma);
    sqlite3_bind_int(stmt, 3, voto);
    sqlite3_bind_text(stmt, 4, commento, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, dataOggiStr, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        snprintf(msgErr, dimErr, "Errore invio feedback");
        return 0;
    }
    return (int)sqlite3_last_insert_rowid(g_db);
}

// ----- caricaFeedbackCliente -----

Feedback* caricaFeedbackCliente(int idCliente, int idProgramma) {
    const char* sql =
        "SELECT id, id_cliente, id_programma, voto, commento, data "
        "FROM feedback WHERE id_cliente = ? AND id_programma = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    sqlite3_bind_int(stmt, 1, idCliente);
    sqlite3_bind_int(stmt, 2, idProgramma);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return NULL;
    }

    Feedback* f = new Feedback();
    f->id = sqlite3_column_int(stmt, 0);
    f->idCliente = sqlite3_column_int(stmt, 1);
    f->idProgramma = sqlite3_column_int(stmt, 2);
    f->voto = sqlite3_column_int(stmt, 3);
    copiaColonnaTesto(stmt, 4, f->commento, sizeof(f->commento));
    copiaColonnaTesto(stmt, 5, f->data, sizeof(f->data));

    sqlite3_finalize(stmt);
    return f;
}

// ----- listaFeedbackProfessionista -----

void listaFeedbackProfessionista(int idProfessionista,
                                 nlohmann::json& destinazione) {
    destinazione = nlohmann::json::array();

    // JOIN: feedback con programmi (per filtrare i miei) e utenti (per nome cliente).
    const char* sql =
        "SELECT f.id, f.voto, f.commento, f.data, "
        "       p.id, p.nome, "
        "       u.nome, u.cognome "
        "FROM feedback f "
        "JOIN programmi p ON p.id = f.id_programma "
        "JOIN utenti u ON u.id = f.id_cliente "
        "WHERE p.id_creatore = ? "
        "ORDER BY f.data DESC, f.id DESC;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return;
    }
    sqlite3_bind_int(stmt, 1, idProfessionista);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        nlohmann::json riga;
        riga["id"] = sqlite3_column_int(stmt, 0);
        riga["voto"] = sqlite3_column_int(stmt, 1);

        char buf[LUNG_COMMENTO_FB];
        copiaColonnaTesto(stmt, 2, buf, sizeof(buf));
        riga["commento"] = buf;

        copiaColonnaTesto(stmt, 3, buf, sizeof(buf));
        riga["data"] = buf;

        riga["idProgramma"] = sqlite3_column_int(stmt, 4);
        copiaColonnaTesto(stmt, 5, buf, sizeof(buf));
        riga["nomeProgramma"] = buf;

        char nome[64], cognome[64];
        copiaColonnaTesto(stmt, 6, nome, sizeof(nome));
        copiaColonnaTesto(stmt, 7, cognome, sizeof(cognome));
        char nomeCompleto[140];
        snprintf(nomeCompleto, sizeof(nomeCompleto), "%s %s", nome, cognome);
        riga["nomeCliente"] = nomeCompleto;

        destinazione.push_back(riga);
    }
    sqlite3_finalize(stmt);
}

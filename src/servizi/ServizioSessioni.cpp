#include "ServizioSessioni.h"

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

static int leggiBooleano(const nlohmann::json& j, const char* chiave) {
    if (j.contains(chiave) && j[chiave].is_boolean()) {
        return j[chiave].get<bool>() ? 1 : 0;
    }
    return 0;
}

static void copiaColonnaTesto(sqlite3_stmt* stmt, int iCol, char* dest, int dimDest) {
    const unsigned char* val = sqlite3_column_text(stmt, iCol);
    if (val == NULL) { dest[0] = '\0'; return; }
    strncpy(dest, (const char*)val, dimDest - 1);
    dest[dimDest - 1] = '\0';
}

// Scrive la data odierna in 'buffer' (lungo >= 11) come "AAAA-MM-GG".
static void dataOggi(char* buffer) {
    time_t t = time(NULL);
    struct tm* info = localtime(&t);
    strftime(buffer, 11, "%Y-%m-%d", info);
}

// ----- registraSessione -----

int registraSessione(int idCliente, const nlohmann::json& dati,
                     char* msgErr, int dimErr) {
    int idProgramma = leggiIntero(dati, "idProgramma");
    int durataMin = leggiIntero(dati, "durataMin");
    int completata = leggiBooleano(dati, "completata");
    char data[LUNG_DATA_SESSIONE];
    char note[LUNG_NOTE_SESSIONE];
    leggiStringa(dati, "data", data, sizeof(data));
    leggiStringa(dati, "note", note, sizeof(note));

    // Se la data non e' stata indicata, uso oggi.
    if (data[0] == '\0') {
        dataOggi(data);
    }

    if (idProgramma <= 0) {
        snprintf(msgErr, dimErr, "idProgramma mancante");
        return 0;
    }
    if (durataMin < 0 || durataMin > 600) {
        snprintf(msgErr, dimErr, "Durata fuori range (0-600 minuti)");
        return 0;
    }

    // Controllo che il programma esista.
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

    const char* sql =
        "INSERT INTO sessioni (id_cliente, id_programma, data, durata_min, completata, note) "
        "VALUES (?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        snprintf(msgErr, dimErr, "Errore interno (prepare)");
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idCliente);
    sqlite3_bind_int(stmt, 2, idProgramma);
    sqlite3_bind_text(stmt, 3, data, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, durataMin);
    sqlite3_bind_int(stmt, 5, completata);
    sqlite3_bind_text(stmt, 6, note, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        snprintf(msgErr, dimErr, "Errore registrazione sessione");
        return 0;
    }
    return (int)sqlite3_last_insert_rowid(g_db);
}

// ----- cancellaSessione -----

int cancellaSessione(int idSessione, int idCliente) {
    const char* sql = "DELETE FROM sessioni WHERE id = ? AND id_cliente = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idSessione);
    sqlite3_bind_int(stmt, 2, idCliente);
    int rc = sqlite3_step(stmt);
    int righe = sqlite3_changes(g_db);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE && righe > 0) ? 1 : 0;
}

// ----- listaSessioniCliente -----

int listaSessioniCliente(int idCliente, Sessione* dest, int maxNum) {
    const char* sql =
        "SELECT id, id_cliente, id_programma, data, durata_min, completata, note "
        "FROM sessioni WHERE id_cliente = ? "
        "ORDER BY data DESC, id DESC;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    sqlite3_bind_int(stmt, 1, idCliente);

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < maxNum) {
        Sessione& s = dest[n];
        s.id = sqlite3_column_int(stmt, 0);
        s.idCliente = sqlite3_column_int(stmt, 1);
        s.idProgramma = sqlite3_column_int(stmt, 2);
        copiaColonnaTesto(stmt, 3, s.data, sizeof(s.data));
        s.durataMin = sqlite3_column_int(stmt, 4);
        s.completata = sqlite3_column_int(stmt, 5);
        copiaColonnaTesto(stmt, 6, s.note, sizeof(s.note));
        n++;
    }
    sqlite3_finalize(stmt);
    return n;
}

// ----- calcolaStatistichePerProfessionista -----

void calcolaStatistichePerProfessionista(int idProfessionista,
                                         nlohmann::json& destinazione) {
    destinazione = nlohmann::json::array();

    // Per ogni programma del professionista, conto sessioni totali e completate.
    // Uso LEFT JOIN cosi' compaiono anche i programmi senza ancora sessioni.
    const char* sql =
        "SELECT p.id, p.nome, "
        "       COUNT(s.id) AS totale, "
        "       SUM(CASE WHEN s.completata = 1 THEN 1 ELSE 0 END) AS completate, "
        "       MAX(s.data) AS ultimaData "
        "FROM programmi p "
        "LEFT JOIN sessioni s ON s.id_programma = p.id "
        "WHERE p.id_creatore = ? "
        "GROUP BY p.id, p.nome "
        "ORDER BY p.id DESC;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return;
    }
    sqlite3_bind_int(stmt, 1, idProfessionista);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        nlohmann::json riga;
        riga["idProgramma"] = sqlite3_column_int(stmt, 0);
        char nome[200];
        copiaColonnaTesto(stmt, 1, nome, sizeof(nome));
        riga["nomeProgramma"] = nome;
        riga["totale"] = sqlite3_column_int(stmt, 2);
        riga["completate"] = sqlite3_column_int(stmt, 3);

        const unsigned char* dataVal = sqlite3_column_text(stmt, 4);
        riga["ultimaData"] = (dataVal == NULL) ? "" : (const char*)dataVal;

        destinazione.push_back(riga);
    }
    sqlite3_finalize(stmt);
}

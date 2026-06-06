#include "Auth.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "persistenza/Database.h"
#include "utils/Hash.h"
#include "utils/Validazione.h"

// ----- Funzioni di supporto -----

// Scrive la data odierna in 'buffer' come "AAAA-MM-GG". Buffer >= 11 char.
static void dataOggi(char* buffer) {
    time_t t = time(NULL);
    struct tm* info = localtime(&t);
    strftime(buffer, 11, "%Y-%m-%d", info);
}

// Genera un token random di LUNGHEZZA_TOKEN caratteri esadecimali.
// srand() viene chiamato una volta in main().
static void generaToken(char* buffer) {
    for (int i = 0; i < LUNGHEZZA_TOKEN; i++) {
        int v = rand() % 16;
        buffer[i] = (v < 10) ? (char)('0' + v) : (char)('a' + v - 10);
    }
    buffer[LUNGHEZZA_TOKEN] = '\0';
}

// Legge una stringa da un campo JSON. Se manca scrive '\0' in 'buffer'.
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

// ----- Registrazione -----

int registra(const nlohmann::json& dati, char* msgErrore, int dimErr) {
    char tipo[LUNG_TIPO];
    char username[LUNG_USERNAME];
    char password[LUNGHEZZA_MAX_PASSWORD + 1];
    char nome[LUNG_NOME];
    char cognome[LUNG_NOME];
    char email[LUNG_EMAIL];

    leggiStringa(dati, "tipo", tipo, sizeof(tipo));
    leggiStringa(dati, "username", username, sizeof(username));
    leggiStringa(dati, "password", password, sizeof(password));
    leggiStringa(dati, "nome", nome, sizeof(nome));
    leggiStringa(dati, "cognome", cognome, sizeof(cognome));
    leggiStringa(dati, "email", email, sizeof(email));

    if (strcmp(tipo, "cliente") != 0 && strcmp(tipo, "professionista") != 0) {
        snprintf(msgErrore, dimErr, "Tipo di account non valido");
        return 0;
    }
    if (!validaUsername(username)) {
        snprintf(msgErrore, dimErr, "Nome utente non valido (3-20 caratteri, solo lettere, numeri e _)");
        return 0;
    }
    if (!validaPassword(password)) {
        snprintf(msgErrore, dimErr, "Password non valida (almeno 6 caratteri)");
        return 0;
    }
    if (!validaNome(nome) || !validaNome(cognome)) {
        snprintf(msgErrore, dimErr, "Nome e cognome obbligatori");
        return 0;
    }
    if (!validaEmail(email)) {
        snprintf(msgErrore, dimErr, "Email non valida");
        return 0;
    }

    // Verifica unicita' della email (anche se username e' diverso).
    {
        const char* sqlEmail = "SELECT id FROM utenti WHERE email = ?;";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(g_db, sqlEmail, -1, &stmt, NULL) != SQLITE_OK) {
            snprintf(msgErrore, dimErr, "Errore interno");
            return 0;
        }
        sqlite3_bind_text(stmt, 1, email, -1, SQLITE_STATIC);
        int emailEsiste = (sqlite3_step(stmt) == SQLITE_ROW);
        sqlite3_finalize(stmt);
        if (emailEsiste) {
            snprintf(msgErrore, dimErr, "Email gia' registrata");
            return 0;
        }
    }

    char hash[LUNGHEZZA_HASH];
    hashPassword(password, hash);

    char dataOggiStr[11];
    dataOggi(dataOggiStr);

    // Campi specifici per il cliente (validati ai limiti).
    int eta = 0;
    double peso = 0.0;
    double altezza = 0.0;
    char obiettivo[LUNG_TIPO];
    obiettivo[0] = '\0';
    if (strcmp(tipo, "cliente") == 0) {
        if (dati.contains("eta") && dati["eta"].is_number()) {
            eta = dati["eta"].get<int>();
            if (eta < 14 || eta > 100) eta = 0;
        }
        if (dati.contains("peso") && dati["peso"].is_number()) {
            peso = dati["peso"].get<double>();
            if (peso < 30 || peso > 250) peso = 0.0;
        }
        if (dati.contains("altezza") && dati["altezza"].is_number()) {
            altezza = dati["altezza"].get<double>();
            if (altezza < 120 || altezza > 230) altezza = 0.0;
        }
        leggiStringa(dati, "obiettivo", obiettivo, sizeof(obiettivo));
        if (strcmp(obiettivo, "perdita_peso") != 0
            && strcmp(obiettivo, "massa") != 0
            && strcmp(obiettivo, "mantenimento") != 0) {
            strcpy(obiettivo, "mantenimento");   // default
        }
    }

    // Specializzazione (solo per professionista). Default "trainer" se non valida.
    char specializzazione[LUNG_TIPO];
    leggiStringa(dati, "specializzazione", specializzazione, sizeof(specializzazione));
    if (strcmp(specializzazione, "trainer") != 0
        && strcmp(specializzazione, "nutrizionista") != 0
        && strcmp(specializzazione, "entrambi") != 0) {
        strcpy(specializzazione, "trainer");
    }

    // Prepared statement: previene SQL injection.
    const char* sql =
        "INSERT INTO utenti (username, password_hash, nome, cognome, email,"
        "                    data_registrazione, tipo, "
        "                    eta, peso, altezza, obiettivo, "
        "                    specializzazione) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        snprintf(msgErrore, dimErr, "Errore interno (prepare)");
        return 0;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hash, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, nome, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, cognome, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, dataOggiStr, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, tipo, -1, SQLITE_STATIC);

    // Posizioni 8-11: campi del cliente. NULL per professionista.
    if (strcmp(tipo, "cliente") == 0) {
        sqlite3_bind_int(stmt, 8, eta);
        sqlite3_bind_double(stmt, 9, peso);
        sqlite3_bind_double(stmt, 10, altezza);
        sqlite3_bind_text(stmt, 11, obiettivo, -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 8);
        sqlite3_bind_null(stmt, 9);
        sqlite3_bind_null(stmt, 10);
        sqlite3_bind_null(stmt, 11);
    }

    // Posizione 12: specializzazione del professionista. NULL per cliente.
    if (strcmp(tipo, "professionista") == 0) {
        sqlite3_bind_text(stmt, 12, specializzazione, -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 12);
    }

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        int errcode = sqlite3_extended_errcode(g_db);
        if (errcode == SQLITE_CONSTRAINT_UNIQUE) {
            snprintf(msgErrore, dimErr, "Nome utente gia' in uso");
        } else {
            snprintf(msgErrore, dimErr, "Errore interno (insert: %s)", sqlite3_errmsg(g_db));
        }
        return 0;
    }

    return (int)sqlite3_last_insert_rowid(g_db);
}

// ----- Login -----

int login(const char* username, const char* password) {
    if (!validaUsername(username) || !validaPassword(password)) {
        return 0;
    }

    char hashAtteso[LUNGHEZZA_HASH];
    hashPassword(password, hashAtteso);

    const char* sql = "SELECT id FROM utenti WHERE username = ? AND password_hash = ?;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashAtteso, -1, SQLITE_STATIC);

    int idTrovato = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        idTrovato = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    return idTrovato;
}

// ----- caricaUtente -----

// Copia la colonna 'iCol' (testo) in 'dest'. NULL nel DB -> stringa vuota.
static void copiaColonnaTesto(sqlite3_stmt* stmt, int iCol, char* dest, int dimDest) {
    const unsigned char* val = sqlite3_column_text(stmt, iCol);
    if (val == NULL) {
        dest[0] = '\0';
        return;
    }
    strncpy(dest, (const char*)val, dimDest - 1);
    dest[dimDest - 1] = '\0';
}

Utente* caricaUtente(int idUtente) {
    const char* sql =
        "SELECT id, username, password_hash, nome, cognome, email, "
        "       data_registrazione, tipo, "
        "       eta, peso, altezza, obiettivo, "
        "       id_programma_corrente, id_piano_corrente, "
        "       id_professionista_assegnato, specializzazione "
        "FROM utenti WHERE id = ?;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    sqlite3_bind_int(stmt, 1, idUtente);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return NULL;
    }

    // 'tipo' determina la sottoclasse da istanziare.
    char tipo[LUNG_TIPO];
    copiaColonnaTesto(stmt, 7, tipo, sizeof(tipo));

    Utente* utente = NULL;

    if (strcmp(tipo, "cliente") == 0) {
        Cliente* c = new Cliente();
        c->eta = sqlite3_column_int(stmt, 8);
        c->peso = sqlite3_column_double(stmt, 9);
        c->altezza = sqlite3_column_double(stmt, 10);
        copiaColonnaTesto(stmt, 11, c->obiettivo, sizeof(c->obiettivo));
        c->idProgrammaCorrente = sqlite3_column_int(stmt, 12);
        c->idPianoCorrente = sqlite3_column_int(stmt, 13);
        c->idProfessionistaAssegnato = sqlite3_column_int(stmt, 14);
        utente = c;
    } else if (strcmp(tipo, "professionista") == 0) {
        Professionista* p = new Professionista();
        copiaColonnaTesto(stmt, 15, p->specializzazione, sizeof(p->specializzazione));
        utente = p;
    } else {
        sqlite3_finalize(stmt);
        return NULL;
    }

    utente->id = sqlite3_column_int(stmt, 0);
    copiaColonnaTesto(stmt, 1, utente->username, sizeof(utente->username));
    copiaColonnaTesto(stmt, 2, utente->passwordHash, sizeof(utente->passwordHash));
    copiaColonnaTesto(stmt, 3, utente->nome, sizeof(utente->nome));
    copiaColonnaTesto(stmt, 4, utente->cognome, sizeof(utente->cognome));
    copiaColonnaTesto(stmt, 5, utente->email, sizeof(utente->email));
    copiaColonnaTesto(stmt, 6, utente->dataRegistrazione, sizeof(utente->dataRegistrazione));

    sqlite3_finalize(stmt);
    return utente;
}

// ----- Sessioni -----

void creaSessione(int idUtente, char* tokenOut) {
    generaToken(tokenOut);

    // Scadenza: timestamp Unix salvato come stringa.
    time_t scadenza = time(NULL) + DURATA_SESSIONE_SEC;
    char scadenzaStr[32];
    snprintf(scadenzaStr, sizeof(scadenzaStr), "%lld", (long long)scadenza);

    const char* sql = "INSERT INTO sessioni_web (token, id_utente, scadenza) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, tokenOut, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, idUtente);
    sqlite3_bind_text(stmt, 3, scadenzaStr, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

int verificaSessione(const char* token) {
    if (token == NULL || token[0] == '\0') {
        return 0;
    }

    const char* sql = "SELECT id_utente, scadenza FROM sessioni_web WHERE token = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    sqlite3_bind_text(stmt, 1, token, -1, SQLITE_STATIC);

    int idUtente = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        idUtente = sqlite3_column_int(stmt, 0);
        const unsigned char* scadenzaStr = sqlite3_column_text(stmt, 1);
        if (scadenzaStr != NULL) {
            time_t scadenza = (time_t)atoll((const char*)scadenzaStr);
            if (time(NULL) > scadenza) {
                idUtente = 0;   // scaduta
            }
        }
    }
    sqlite3_finalize(stmt);
    return idUtente;
}

void rimuoviSessione(const char* token) {
    if (token == NULL || token[0] == '\0') {
        return;
    }
    const char* sql = "DELETE FROM sessioni_web WHERE token = ?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, token, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

int aggiornaProfilo(int idCliente, const nlohmann::json& dati, char* msgErr, int dimErr) {
    if (!dati.contains("eta") || !dati["eta"].is_number()) {
        snprintf(msgErr, dimErr, "eta mancante");
        return 0;
    }
    if (!dati.contains("peso") || !dati["peso"].is_number()) {
        snprintf(msgErr, dimErr, "peso mancante");
        return 0;
    }
    if (!dati.contains("altezza") || !dati["altezza"].is_number()) {
        snprintf(msgErr, dimErr, "altezza mancante");
        return 0;
    }

    int eta = dati["eta"].get<int>();
    double peso = dati["peso"].get<double>();
    double altezza = dati["altezza"].get<double>();

    if (eta < 14 || eta > 100) {
        snprintf(msgErr, dimErr, "Eta' fuori range (14-100)");
        return 0;
    }
    if (peso < 30.0 || peso > 250.0) {
        snprintf(msgErr, dimErr, "Peso fuori range (30-250 kg)");
        return 0;
    }
    if (altezza < 120.0 || altezza > 230.0) {
        snprintf(msgErr, dimErr, "Altezza fuori range (120-230 cm)");
        return 0;
    }

    const char* sql =
        "UPDATE utenti SET eta = ?, peso = ?, altezza = ? "
        "WHERE id = ? AND tipo = 'cliente';";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        snprintf(msgErr, dimErr, "Errore interno");
        return 0;
    }
    sqlite3_bind_int(stmt, 1, eta);
    sqlite3_bind_double(stmt, 2, peso);
    sqlite3_bind_double(stmt, 3, altezza);
    sqlite3_bind_int(stmt, 4, idCliente);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        snprintf(msgErr, dimErr, "Errore aggiornamento profilo");
        return 0;
    }
    return 1;
}

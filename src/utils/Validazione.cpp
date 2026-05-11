#include "Validazione.h"

#include <cstring>
#include <cctype>

int validaStringa(const char* str, int lunghezzaMin, int lunghezzaMax) {
    if (str == NULL) {
        return 0;
    }
    int lunghezza = (int)strlen(str);
    if (lunghezza < lunghezzaMin || lunghezza > lunghezzaMax) {
        return 0;
    }
    return 1;
}

int validaUsername(const char* username) {
    if (!validaStringa(username, LUNGHEZZA_MIN_USERNAME, LUNGHEZZA_MAX_USERNAME)) {
        return 0;
    }
    // Scorriamo tutti i caratteri e accettiamo solo lettere, numeri e '_'.
    // per evitare sorprese caratteri accentati: facciamo il cast per sicurezza.
    for (int i = 0; username[i] != '\0'; i++) {
        char c = username[i];
        if (!isalnum((unsigned char)c) && c != '_') {
            return 0;
        }
    }
    return 1;
}

int validaPassword(const char* password) {
    return validaStringa(password, LUNGHEZZA_MIN_PASSWORD, LUNGHEZZA_MAX_PASSWORD);
}

int validaEmail(const char* email) {
    if (!validaStringa(email, LUNGHEZZA_MIN_EMAIL, LUNGHEZZA_MAX_EMAIL)) {
        return 0;
    }
    // Controllo "minimale ma utile": deve avere almeno una '@' e un '.'.
    // Validare l'email in modo rigoroso e' molto piu' complesso, ma per
    // un caso di studio questa verifica e' sufficiente.
    int trovataChiocciola = 0;
    int trovatoPunto = 0;
    for (int i = 0; email[i] != '\0'; i++) {
        if (email[i] == '@') trovataChiocciola = 1;
        if (email[i] == '.') trovatoPunto = 1;
    }
    if (!trovataChiocciola || !trovatoPunto) {
        return 0;
    }
    return 1;
}

int validaNome(const char* nome) {
    return validaStringa(nome, 1, LUNGHEZZA_MAX_NOME);
}

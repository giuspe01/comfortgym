#include "Hash.h"

#include <cstdio>

void hashPassword(const char* password, char* buffer) {
    // Valore iniziale raccomandato dall'autore dell'algoritmo.
    unsigned long hash = 5381;

    int c;
    while ((c = *password) != '\0') {
        hash = ((hash << 5) + hash) + (unsigned long)c;
        password++;
    }

    // snprintf garantisce di non scrivere oltre la dimensione massima.
    snprintf(buffer, LUNGHEZZA_HASH, "%lu", hash);
}

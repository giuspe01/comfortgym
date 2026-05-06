#include "Hash.h"

#include <cstdio>

void hashPassword(const char* password, char* buffer) {
    // Valore iniziale "magico" raccomandato dall'autore dell'algoritmo.
    unsigned long hash = 5381;

    // Per ogni carattere della stringa: hash = hash * 33 + carattere.
    // L'espressione (hash << 5) + hash equivale a hash * 32 + hash = hash * 33,
    // ma e' leggermente piu' veloce sui processori vecchi.
    int c;
    while ((c = *password) != '\0') {
        hash = ((hash << 5) + hash) + (unsigned long)c;
        password++;
    }

    // Scriviamo il numero come stringa decimale dentro il buffer.
    // snprintf garantisce di non scrivere oltre la dimensione massima.
    snprintf(buffer, LUNGHEZZA_HASH, "%lu", hash);
}

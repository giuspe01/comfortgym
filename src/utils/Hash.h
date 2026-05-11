// Hash.h - Funzione di hash.
//
// Algoritmo usato: djb2 di Daniel J. Bernstein. Non consigliato, si dovrebbe usare
// bcrypt/argon2 o altri.

#ifndef PALESTRA_HASH_H
#define PALESTRA_HASH_H

// Lunghezza massima del buffer in cui scrivere l'hash come stringa.
// Un unsigned long ha al piu' 20 cifre decimali + il terminatore '\0'.
#define LUNGHEZZA_HASH 32

// Calcola l'hash djb2 della stringa 'password'
void hashPassword(const char* password, char* buffer);

#endif

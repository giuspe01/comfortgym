// Hash.h - Funzione di hash per le password.
//
// Lo scopo e' non salvare mai le password in chiaro nel database:
// salviamo solo il loro "hash", cioe' un numero che la rappresenta in modo
// univoco e che non si puo' (facilmente) invertire.
//
// Algoritmo usato: djb2 di Daniel J. Bernstein. E' un hash molto semplice,
// non crittograficamente robusto: per un progetto reale si userebbe
// bcrypt/argon2, ma per il caso di studio e' piu' che sufficiente.

#ifndef PALESTRA_HASH_H
#define PALESTRA_HASH_H

// Lunghezza massima del buffer in cui scrivere l'hash come stringa.
// Un unsigned long ha al piu' 20 cifre decimali + il terminatore '\0'.
#define LUNGHEZZA_HASH 32

// Calcola l'hash djb2 della stringa 'password' e lo scrive in 'buffer'
// come stringa decimale. Il chiamante deve garantire che 'buffer'
// abbia almeno LUNGHEZZA_HASH caratteri disponibili.
void hashPassword(const char* password, char* buffer);

#endif

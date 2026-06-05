// ServizioProgrammi.h - CRUD per programmi ed esercizi, selezione cliente.

#ifndef PALESTRA_SERVIZIOPROGRAMMI_H
#define PALESTRA_SERVIZIOPROGRAMMI_H

#include "json.hpp"
#include "modello/Programma.h"

#define MAX_PROGRAMMI_PER_QUERY 100

// Crea un programma con i suoi esercizi (transazione). Ritorna l'id assegnato
// al programma, 0 in caso di errore (scrive la causa in msgErr).
int creaProgramma(const nlohmann::json& dati, int idCreatore,
                  char* msgErr, int dimErr);

// Aggiorna un programma esistente. Sostituisce anche tutti gli esercizi.
// Solo l'utente che ha creato il programma puo' modificarlo.
// Ritorna 1 se ok, 0 in caso di errore.
int aggiornaProgramma(int idProgramma, const nlohmann::json& dati,
                      int idCreatore, char* msgErr, int dimErr);

// Cancella un programma. Solo il creatore. Gli esercizi vengono cancellati
// automaticamente per via di ON DELETE CASCADE.
int cancellaProgramma(int idProgramma, int idCreatore);

// Carica un programma dal DB. NULL se non esiste. Caller deve fare delete.
Programma* caricaProgramma(int idProgramma);

// Carica gli esercizi di un programma. Riempie 'dest' (max 'maxNum').
// Ritorna il numero di esercizi scritti.
int caricaEsercizi(int idProgramma, Esercizio* dest, int maxNum);

// Lista programmi con filtri opzionali (passa NULL/0 per non filtrare).
int listaProgrammi(Programma* dest, int maxNum,
                   const char* obiettivo,
                   const char* difficolta,
                   int durataMax);

// Lista i programmi creati da un utente.
int listaProgrammiDelCreatore(int idCreatore, Programma* dest, int maxNum);

// Imposta il programma corrente del cliente. idProgramma=0 = nessuno.
// Verifica che il programma esista. Ritorna 1 se ok, 0 in caso di errore.
int selezionaProgrammaCliente(int idCliente, int idProgramma);

#endif

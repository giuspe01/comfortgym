// ServizioAderenza.h - Registrazione e lettura dell'aderenza nutrizionale.

#ifndef PALESTRA_SERVIZIOADERENZA_H
#define PALESTRA_SERVIZIOADERENZA_H

#include "json.hpp"
#include "modello/Aderenza.h"

#define MAX_ADERENZE_PER_QUERY 200

// Registra (o sovrascrive) l'aderenza del cliente per il giorno indicato.
// Una sola registrazione per giorno: re-inviarla aggiorna il valore.
// Ritorna l'id, 0 in caso di errore.
int registraAderenza(int idCliente, const nlohmann::json& dati,
                     char* msgErr, int dimErr);

// Cancella una registrazione di aderenza (solo del cliente).
int cancellaAderenza(int idAderenza, int idCliente);

// Lista le registrazioni di aderenza di un cliente, dalla piu' recente.
int listaAderenzeCliente(int idCliente, Aderenza* dest, int maxNum);

// Aggiorna il consiglio del nutrizionista su una registrazione di aderenza.
// Ritorna 1 se aggiornato, 0 se l'aderenza non appartiene a un piano del nutrizionista.
int aggiornaConsiglioPro(int idAderenza, const char* consiglio, int idNutrizionista);

// Riempie 'destinazione' con le aderenze di tutti i clienti sui piani del nutrizionista.
void listaAderenzePerNutrizionista(int idNutrizionista, nlohmann::json& destinazione);

// Riempie 'destinazione' con le statistiche aggregate per ogni piano del nutrizionista.
void attivitaAgregatePiani(int idNutrizionista, nlohmann::json& destinazione);

#endif

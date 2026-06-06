// ServizioSessioni.h - Registrazione e lettura delle sessioni di allenamento.

#ifndef PALESTRA_SERVIZIOSESSIONI_H
#define PALESTRA_SERVIZIOSESSIONI_H

#include "json.hpp"
#include "modello/Sessione.h"

#define MAX_SESSIONI_PER_QUERY 200

// Registra una sessione svolta da 'idCliente'.
// Ritorna l'id della sessione creata, 0 in caso di errore.
int registraSessione(int idCliente, const nlohmann::json& dati,
                     char* msgErr, int dimErr);

// Cancella una sessione. Solo il cliente che l'ha registrata.
int cancellaSessione(int idSessione, int idCliente);

// Lista le sessioni di un cliente (piu' recenti prima).
int listaSessioniCliente(int idCliente, Sessione* dest, int maxNum);

// Statistiche aggregate per i programmi creati da un professionista:
// per ogni programma del professionista riempie un JSON con
//   { idProgramma, nomeProgramma, totale, completate, ultimaData }.
// Riempie 'destinazione' come json::array.
void calcolaStatistichePerProfessionista(int idProfessionista,
                                         nlohmann::json& destinazione);

// Lista le singole sessioni svolte sui programmi di un professionista,
// includendo nome, email e dati anagrafici del cliente.
// Ogni riga: { idSessione, idProgramma, nomeProgramma, idCliente,
//              nomeCliente, emailCliente, etaCliente, pesoCliente,
//              altezzaCliente, obiettivoCliente, data, durataMin,
//              completata, note }.
void listaSessioniPerProfessionista(int idProfessionista,
                                    nlohmann::json& destinazione);

#endif

// ServizioFeedback.h - Invio e lettura dei feedback sui programmi.

#ifndef PALESTRA_SERVIZIOFEEDBACK_H
#define PALESTRA_SERVIZIOFEEDBACK_H

#include "json.hpp"
#include "modello/Feedback.h"

// Invia o aggiorna un feedback del cliente su un programma.
// Se il cliente aveva gia' lasciato un feedback per lo stesso programma,
// viene aggiornato (voto+commento sovrascritti). Ritorna l'id del feedback,
// 0 in caso di errore.
int inviaFeedback(int idCliente, const nlohmann::json& dati,
                  char* msgErr, int dimErr);

// Carica il feedback del cliente su un programma specifico, se esiste.
// NULL altrimenti. Il chiamante deve fare delete.
Feedback* caricaFeedbackCliente(int idCliente, int idProgramma);

// Riempie 'destinazione' con tutti i feedback ricevuti sui programmi creati
// dal professionista, inclusi nome cliente e nome programma. JSON array.
void listaFeedbackProfessionista(int idProfessionista,
                                 nlohmann::json& destinazione);

#endif

// ServizioPiani.h - CRUD per piani nutrizionali e pasti, selezione cliente.

#ifndef PALESTRA_SERVIZIOPIANI_H
#define PALESTRA_SERVIZIOPIANI_H

#include "json.hpp"
#include "modello/PianoNutrizionale.h"

#define MAX_PIANI_PER_QUERY 100

int creaPiano(const nlohmann::json& dati, int idCreatore,
              char* msgErr, int dimErr);

int aggiornaPiano(int idPiano, const nlohmann::json& dati,
                  int idCreatore, char* msgErr, int dimErr);

int cancellaPiano(int idPiano, int idCreatore);

PianoNutrizionale* caricaPiano(int idPiano);

int caricaPasti(int idPiano, Pasto* dest, int maxNum);

// Lista piani con filtro opzionale (calorieMax=0 = nessun filtro).
int listaPiani(PianoNutrizionale* dest, int maxNum, int calorieMax);

int listaPianiDelCreatore(int idCreatore, PianoNutrizionale* dest, int maxNum);

// Imposta il piano corrente del cliente. idPiano=0 = nessuno.
int selezionaPianoCliente(int idCliente, int idPiano);

#endif

// PianoNutrizionale.h - Modello dati per piani nutrizionali e pasti.

#ifndef PALESTRA_PIANONUTRIZIONALE_H
#define PALESTRA_PIANONUTRIZIONALE_H

#include "json.hpp"

#define LUNG_NOME_PIANO     100
#define LUNG_TIPO_PASTO      32       // "colazione" | "pranzo" | "cena" | "spuntino"
#define LUNG_DESCRIZIONE    300

#define MAX_PASTI_PER_PIANO  20

class Pasto {
public:
    int id;
    int idPiano;
    char tipo[LUNG_TIPO_PASTO];
    char descrizione[LUNG_DESCRIZIONE];
    int calorie;

    Pasto();
    nlohmann::json toJson() const;
};

class PianoNutrizionale {
public:
    int id;
    char nome[LUNG_NOME_PIANO];
    int calorieGiornaliere;
    int idCreatore;

    PianoNutrizionale();
    nlohmann::json toJson() const;
};

#endif

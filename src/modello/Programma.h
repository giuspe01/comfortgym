// Programma.h - Modello dati per programmi di allenamento ed esercizi.

#ifndef PALESTRA_PROGRAMMA_H
#define PALESTRA_PROGRAMMA_H

#include "json.hpp"

#define LUNG_NOME_PROGRAMMA   100
#define LUNG_OBIETTIVO         32
#define LUNG_DIFFICOLTA        32
#define LUNG_NOME_ESERCIZIO   100
#define LUNG_NOTE             300

#define MAX_ESERCIZI_PER_PROGRAMMA 20

class Esercizio {
public:
    int id;
    int idProgramma;
    char nome[LUNG_NOME_ESERCIZIO];
    int serie;
    int ripetizioni;
    int riposoSec;
    char note[LUNG_NOTE];

    Esercizio();
    nlohmann::json toJson() const;
};

class Programma {
public:
    int id;
    char nome[LUNG_NOME_PROGRAMMA];
    char obiettivo[LUNG_OBIETTIVO];      // "perdita_peso" | "massa" | "mantenimento"
    char difficolta[LUNG_DIFFICOLTA];    // "principiante" | "intermedio" | "avanzato"
    int durataSettimane;
    int idCreatore;

    Programma();
    nlohmann::json toJson() const;
};

#endif

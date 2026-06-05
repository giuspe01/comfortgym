// Sessione.h - Sessione di allenamento svolta da un cliente.

#ifndef PALESTRA_SESSIONE_H
#define PALESTRA_SESSIONE_H

#include "json.hpp"

#define LUNG_DATA_SESSIONE  16     // "AAAA-MM-GG"
#define LUNG_NOTE_SESSIONE  300

class Sessione {
public:
    int id;
    int idCliente;
    int idProgramma;
    char data[LUNG_DATA_SESSIONE];
    int durataMin;
    int completata;                // 0 o 1
    char note[LUNG_NOTE_SESSIONE];

    Sessione();
    nlohmann::json toJson() const;
};

#endif

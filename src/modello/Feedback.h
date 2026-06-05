// Feedback.h - Voto e commento di un cliente su un programma.

#ifndef PALESTRA_FEEDBACK_H
#define PALESTRA_FEEDBACK_H

#include "json.hpp"

#define LUNG_COMMENTO_FB  500
#define LUNG_DATA_FB       16

class Feedback {
public:
    int id;
    int idCliente;
    int idProgramma;
    int voto;                     // 1..5
    char commento[LUNG_COMMENTO_FB];
    char data[LUNG_DATA_FB];

    Feedback();
    nlohmann::json toJson() const;
};

#endif

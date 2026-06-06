// FeedbackPiano.h - Voto e commento di un cliente su un piano nutrizionale.

#ifndef PALESTRA_FEEDBACKPIANO_H
#define PALESTRA_FEEDBACKPIANO_H

#include "json.hpp"

#define LUNG_COMMENTO_FBP  500
#define LUNG_DATA_FBP       16

class FeedbackPiano {
public:
    int id;
    int idCliente;
    int idPiano;
    int voto;                          // 1..5
    char commento[LUNG_COMMENTO_FBP];
    char data[LUNG_DATA_FBP];

    FeedbackPiano();
    nlohmann::json toJson() const;
};

#endif

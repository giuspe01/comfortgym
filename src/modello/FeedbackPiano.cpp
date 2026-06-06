#include "FeedbackPiano.h"

FeedbackPiano::FeedbackPiano() {
    id = 0;
    idCliente = 0;
    idPiano = 0;
    voto = 0;
    commento[0] = '\0';
    data[0] = '\0';
}

nlohmann::json FeedbackPiano::toJson() const {
    nlohmann::json j;
    j["id"] = id;
    j["idCliente"] = idCliente;
    j["idPiano"] = idPiano;
    j["voto"] = voto;
    j["commento"] = commento;
    j["data"] = data;
    return j;
}

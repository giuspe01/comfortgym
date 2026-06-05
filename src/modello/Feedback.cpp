#include "Feedback.h"

Feedback::Feedback() {
    id = 0;
    idCliente = 0;
    idProgramma = 0;
    voto = 0;
    commento[0] = '\0';
    data[0] = '\0';
}

nlohmann::json Feedback::toJson() const {
    nlohmann::json j;
    j["id"] = id;
    j["idCliente"] = idCliente;
    j["idProgramma"] = idProgramma;
    j["voto"] = voto;
    j["commento"] = commento;
    j["data"] = data;
    return j;
}

#include "Sessione.h"

Sessione::Sessione() {
    id = 0;
    idCliente = 0;
    idProgramma = 0;
    nomeProgramma[0] = '\0';
    data[0] = '\0';
    durataMin = 0;
    completata = 0;
    note[0] = '\0';
}

nlohmann::json Sessione::toJson() const {
    nlohmann::json j;
    j["id"] = id;
    j["idCliente"] = idCliente;
    j["idProgramma"] = idProgramma;
    j["nomeProgramma"] = nomeProgramma;
    j["data"] = data;
    j["durataMin"] = durataMin;
    j["completata"] = (completata != 0);
    j["note"] = note;
    return j;
}

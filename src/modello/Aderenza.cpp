#include "Aderenza.h"

Aderenza::Aderenza() {
    id = 0;
    idCliente = 0;
    idPiano = 0;
    nomePiano[0] = '\0';
    data[0] = '\0';
    percentuale = 0;
    note[0] = '\0';
    consiglioPro[0] = '\0';
}

nlohmann::json Aderenza::toJson() const {
    nlohmann::json j;
    j["id"] = id;
    j["idCliente"] = idCliente;
    j["idPiano"] = idPiano;
    j["nomePiano"] = nomePiano;
    j["data"] = data;
    j["percentuale"] = percentuale;
    j["note"] = note;
    j["consiglioPro"] = consiglioPro;
    return j;
}

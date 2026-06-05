#include "Programma.h"

Esercizio::Esercizio() {
    id = 0;
    idProgramma = 0;
    nome[0] = '\0';
    serie = 0;
    ripetizioni = 0;
    riposoSec = 0;
    note[0] = '\0';
}

nlohmann::json Esercizio::toJson() const {
    nlohmann::json j;
    j["id"] = id;
    j["nome"] = nome;
    j["serie"] = serie;
    j["ripetizioni"] = ripetizioni;
    j["riposoSec"] = riposoSec;
    j["note"] = note;
    return j;
}

Programma::Programma() {
    id = 0;
    nome[0] = '\0';
    obiettivo[0] = '\0';
    difficolta[0] = '\0';
    durataSettimane = 0;
    idCreatore = 0;
}

nlohmann::json Programma::toJson() const {
    nlohmann::json j;
    j["id"] = id;
    j["nome"] = nome;
    j["obiettivo"] = obiettivo;
    j["difficolta"] = difficolta;
    j["durataSettimane"] = durataSettimane;
    j["idCreatore"] = idCreatore;
    return j;
}

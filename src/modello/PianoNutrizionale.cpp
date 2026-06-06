#include "PianoNutrizionale.h"

Pasto::Pasto() {
    id = 0;
    idPiano = 0;
    tipo[0] = '\0';
    descrizione[0] = '\0';
    calorie = 0;
}

nlohmann::json Pasto::toJson() const {
    nlohmann::json j;
    j["id"] = id;
    j["tipo"] = tipo;
    j["descrizione"] = descrizione;
    j["calorie"] = calorie;
    return j;
}

PianoNutrizionale::PianoNutrizionale() {
    id = 0;
    nome[0] = '\0';
    calorieGiornaliere = 0;
    idCreatore = 0;
    consigliGiornalieri[0] = '\0';
    immagine[0] = '\0';
}

nlohmann::json PianoNutrizionale::toJson() const {
    nlohmann::json j;
    j["id"] = id;
    j["nome"] = nome;
    j["calorieGiornaliere"] = calorieGiornaliere;
    j["idCreatore"] = idCreatore;
    j["consigliGiornalieri"] = consigliGiornalieri;
    j["immagine"] = immagine;
    return j;
}

#include "Utente.h"

#include <cstring>

// Inizializzazione esplicita: senza, i campi conterrebbero valori indefiniti.

Utente::Utente() {
    id = 0;
    username[0] = '\0';
    passwordHash[0] = '\0';
    nome[0] = '\0';
    cognome[0] = '\0';
    email[0] = '\0';
    dataRegistrazione[0] = '\0';
}

nlohmann::json Utente::toJson() const {
    nlohmann::json j;
    j["id"] = id;
    j["username"] = username;
    j["nome"] = nome;
    j["cognome"] = cognome;
    j["email"] = email;
    j["dataRegistrazione"] = dataRegistrazione;
    j["tipo"] = tipo();
    // passwordHash escluso volutamente.
    return j;
}

Cliente::Cliente() : Utente() {
    eta = 0;
    peso = 0.0;
    altezza = 0.0;
    obiettivo[0] = '\0';
    idProgrammaCorrente = 0;
    idPianoCorrente = 0;
    idProfessionistaAssegnato = 0;
}

nlohmann::json Cliente::toJson() const {
    nlohmann::json j = Utente::toJson();
    j["eta"] = eta;
    j["peso"] = peso;
    j["altezza"] = altezza;
    j["obiettivo"] = obiettivo;
    j["idProgrammaCorrente"] = idProgrammaCorrente;
    j["idPianoCorrente"] = idPianoCorrente;
    j["idProfessionistaAssegnato"] = idProfessionistaAssegnato;
    return j;
}

Professionista::Professionista() : Utente() {
    specializzazione[0] = '\0';
}

nlohmann::json Professionista::toJson() const {
    nlohmann::json j = Utente::toJson();
    j["specializzazione"] = specializzazione;
    return j;
}

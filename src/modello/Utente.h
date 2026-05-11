// Utente.h - Modello dati degli utenti.
//
// Gerarchia: Utente (astratta) -> Cliente | Professionista.

#ifndef PALESTRA_UTENTE_H
#define PALESTRA_UTENTE_H

#include "json.hpp"
#include "utils/Hash.h"
#include "utils/Validazione.h"

// Lunghezze massime dei campi (+1 per il terminatore '\0').
#define LUNG_USERNAME (LUNGHEZZA_MAX_USERNAME + 1)
#define LUNG_NOME     (LUNGHEZZA_MAX_NOME + 1)
#define LUNG_EMAIL    (LUNGHEZZA_MAX_EMAIL + 1)
#define LUNG_DATA     16
#define LUNG_TIPO     32

class Utente {
public:
    int id;
    char username[LUNG_USERNAME];
    char passwordHash[LUNGHEZZA_HASH];
    char nome[LUNG_NOME];
    char cognome[LUNG_NOME];
    char email[LUNG_EMAIL];
    char dataRegistrazione[LUNG_DATA];

    Utente();
    // Distruttore virtuale: necessario per la deallocazione corretta via Utente*.
    virtual ~Utente() {}

    // Virtuale pura: ogni derivata deve implementarla.
    virtual const char* tipo() const = 0;

    // Serializza i campi comuni. Le derivate aggiungono i propri.
    virtual nlohmann::json toJson() const;
};

class Cliente : public Utente {
public:
    int eta;
    double peso;
    double altezza;
    char obiettivo[LUNG_TIPO];               // "perdita_peso" | "massa" | "mantenimento"
    int idProgrammaCorrente;                 // 0 = nessuno
    int idPianoCorrente;                     // 0 = nessuno
    int idProfessionistaAssegnato;           // 0 = nessuno

    Cliente();

    const char* tipo() const override { return "cliente"; }
    nlohmann::json toJson() const override;
};

class Professionista : public Utente {
public:
    char specializzazione[LUNG_TIPO];        // "trainer" | "nutrizionista" | "entrambi"

    Professionista();

    const char* tipo() const override { return "professionista"; }
    nlohmann::json toJson() const override;
};

#endif

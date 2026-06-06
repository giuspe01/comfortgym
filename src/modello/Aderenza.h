// Aderenza.h - Registrazione giornaliera dell'aderenza al piano nutrizionale.

#ifndef PALESTRA_ADERENZA_H
#define PALESTRA_ADERENZA_H

#include "json.hpp"

#define LUNG_DATA_ADERENZA    16
#define LUNG_NOTE_ADERENZA   300
#define LUNG_NOME_ADERENZA   100    // nome piano al momento della registrazione
#define LUNG_CONSIGLIO_PRO   300    // consiglio del nutrizionista su quella giornata

class Aderenza {
public:
    int id;
    int idCliente;
    int idPiano;
    char nomePiano[LUNG_NOME_ADERENZA];
    char data[LUNG_DATA_ADERENZA];
    int percentuale;                       // 0..100
    char note[LUNG_NOTE_ADERENZA];
    char consiglioPro[LUNG_CONSIGLIO_PRO];

    Aderenza();
    nlohmann::json toJson() const;
};

#endif

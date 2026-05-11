#include "HttpUtils.h"

#include <exception>

void rispondiOk(httplib::Response& res, const nlohmann::json& dati) {
    res.status = 200;
    // .dump() trasforma il JSON in stringa.
    res.set_content(dati.dump(), "application/json");
}

void rispondiErrore(httplib::Response& res, int statusHttp, const char* messaggio) {
    nlohmann::json corpo;
    corpo["errore"] = messaggio;
    res.status = statusHttp;
    res.set_content(corpo.dump(), "application/json");
}

int leggiCorpoJson(const httplib::Request& req,
                   httplib::Response& res,
                   nlohmann::json& destinazione) {
    // nlohmann::json::parse lancia un'eccezione se la stringa non e' un JSON
    // valido.
    try {
        destinazione = nlohmann::json::parse(req.body);
        return 1;
    } catch (const std::exception&) {
        rispondiErrore(res, 400, "Corpo della richiesta non e' un JSON valido");
        return 0;
    }
}

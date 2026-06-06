#include "UploadController.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdlib>

#include "controller/AuthController.h"
#include "utils/HttpUtils.h"
#include "json.hpp"

void registraRotteUpload(httplib::Server& server) {
    server.Post("/api/upload",
        [](const httplib::Request& req, httplib::Response& res) {
            int idUtente = idUtenteCorrente(req);
            if (idUtente == 0) {
                rispondiErrore(res, 401, "Non autenticato");
                return;
            }

            auto it = req.files.find("file");
            if (it == req.files.end()) {
                rispondiErrore(res, 400, "Nessun file ricevuto");
                return;
            }
            const httplib::MultipartFormData& f = it->second;

            // Estrai estensione dal nome originale; default .jpg se non riconosciuta.
            char ext[16];
            strcpy(ext, ".jpg");
            size_t punto = f.filename.rfind('.');
            if (punto != std::string::npos) {
                std::string e = f.filename.substr(punto);
                if (e == ".jpg" || e == ".jpeg" || e == ".png" ||
                    e == ".webp" || e == ".gif") {
                    strncpy(ext, e.c_str(), 15);
                    ext[15] = '\0';
                }
            }

            // Nome file univoco: timestamp + numero casuale.
            long ts = (long)time(NULL);
            int rnd = rand() % 9999;

            char percorso[300];
            char url[200];
            sprintf(percorso, "public/uploads/%ld_%d%s", ts, rnd, ext);
            sprintf(url,      "/uploads/%ld_%d%s",       ts, rnd, ext);

            FILE* fp = fopen(percorso, "wb");
            if (!fp) {
                rispondiErrore(res, 500, "Errore salvataggio file");
                return;
            }
            fwrite(f.content.c_str(), 1, f.content.size(), fp);
            fclose(fp);

            nlohmann::json corpo;
            corpo["url"] = url;
            rispondiOk(res, corpo);
        });
}

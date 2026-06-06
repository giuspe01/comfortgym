// catalogo-programmi-pro.js - Gestione programmi del professionista (card + CRUD).

let modalDettaglio = null;

window.addEventListener("DOMContentLoaded", async function () {
    const utente = await richiediUtente("professionista");
    if (!utente) return;

    // Solo trainer e entrambi possono gestire i programmi.
    const spec = utente.specializzazione || "entrambi";
    if (spec === "nutrizionista") {
        window.location.href = "/professionista.html";
        return;
    }

    // Mostra il link ai piani solo se il professionista ha quella specializzazione.
    if (spec === "entrambi") {
        document.getElementById("linkPiani").classList.remove("d-none");
    }

    modalDettaglio = new bootstrap.Modal(document.getElementById("modalDettaglio"));

    document.getElementById("btnLogout").addEventListener("click", logout);
    document.getElementById("btnNuovoProgramma").addEventListener("click", apriFormNuovo);
    document.getElementById("btnAnnullaProgramma").addEventListener("click", chiudiForm);
    document.getElementById("btnAggiungiEsercizio").addEventListener("click", function () {
        aggiungiRiga();
    });
    document.getElementById("formProgramma").addEventListener("submit", salvaProgramma);

    await caricaGriglia();
});

// ----- Griglia card -----

async function caricaGriglia() {
    const griglia = document.getElementById("griglia");
    griglia.innerHTML = '<div class="col"><p class="text-muted">Caricamento...</p></div>';

    const r = await apiGet("/api/professionista/programmi");
    if (!r.ok) {
        griglia.innerHTML = '<div class="col"><p class="text-danger">Errore caricamento.</p></div>';
        return;
    }

    const programmi = r.dati.programmi;
    if (programmi.length === 0) {
        griglia.innerHTML = '<div class="col"><p class="text-muted">Non hai ancora creato programmi.</p></div>';
        return;
    }

    let html = "";
    for (const p of programmi) {
        let imgHtml;
        if (p.immagine && p.immagine.length > 0) {
            imgHtml = '<img src="' + escapeAttr(p.immagine) + '" class="card-img-top card-img-programma" alt="' + escapeAttr(p.nome) + '">';
        } else {
            imgHtml = '<div class="card-img-placeholder">&#x1F3CB;</div>';
        }

        html += '<div class="col">';
        html += '<div class="card h-100 shadow-sm">';
        html += imgHtml;
        html += '<div class="card-body">';
        html += '<h5 class="card-title">' + escapeHtml(p.nome) + '</h5>';
        html += '<p class="card-text small text-muted mb-1"><strong>Obiettivo:</strong> ' + escapeHtml(p.obiettivo) + '</p>';
        html += '<p class="card-text small text-muted mb-0">';
        html += '<span class="badge bg-light text-dark border me-1">' + escapeHtml(p.difficolta) + '</span>';
        html += '<span class="badge bg-light text-dark border">' + p.durataSettimane + ' sett.</span>';
        html += '</p>';
        html += '</div>';
        html += '<div class="card-footer d-flex gap-1 justify-content-end">';
        html += '<button class="btn btn-sm btn-outline-secondary" data-dettaglio="' + p.id + '">Dettaglio</button>';
        html += '<button class="btn btn-sm btn-outline-primary" data-modifica="' + p.id + '">Modifica</button>';
        html += '<button class="btn btn-sm btn-outline-danger" data-elimina="' + p.id + '">Elimina</button>';
        html += '</div>';
        html += '</div>';
        html += '</div>';
    }
    griglia.innerHTML = html;

    griglia.querySelectorAll("button[data-dettaglio]").forEach(function (btn) {
        btn.addEventListener("click", function () {
            apriDettaglio(parseInt(btn.getAttribute("data-dettaglio")));
        });
    });
    griglia.querySelectorAll("button[data-modifica]").forEach(function (btn) {
        btn.addEventListener("click", function () {
            apriFormModifica(parseInt(btn.getAttribute("data-modifica")));
        });
    });
    griglia.querySelectorAll("button[data-elimina]").forEach(function (btn) {
        btn.addEventListener("click", function () {
            eliminaProgramma(parseInt(btn.getAttribute("data-elimina")));
        });
    });
}

// ----- Modal dettaglio -----

async function apriDettaglio(id) {
    const r = await apiGet("/api/programmi/" + id);
    if (!r.ok) { alert("Impossibile caricare il programma"); return; }

    const p = r.dati.programma;
    const esercizi = r.dati.esercizi;

    document.getElementById("modalTitolo").textContent = p.nome;

    let corpo = '<p class="mb-1"><strong>Obiettivo:</strong> ' + escapeHtml(p.obiettivo) + '</p>';
    corpo += '<p class="mb-1"><strong>Difficolta\':</strong> ' + escapeHtml(p.difficolta) + '</p>';
    corpo += '<p class="mb-3"><strong>Durata:</strong> ' + p.durataSettimane + ' settimane</p>';

    if (esercizi.length === 0) {
        corpo += '<p class="text-muted">Nessun esercizio definito.</p>';
    } else {
        corpo += '<h6>Esercizi</h6>';
        corpo += '<table class="table table-sm">';
        corpo += '<thead><tr><th>Esercizio</th><th>Serie</th><th>Rip.</th><th>Riposo</th><th>Note</th></tr></thead><tbody>';
        for (const e of esercizi) {
            corpo += '<tr>';
            corpo += '<td>' + escapeHtml(e.nome) + '</td>';
            corpo += '<td>' + e.serie + '</td>';
            corpo += '<td>' + e.ripetizioni + '</td>';
            corpo += '<td>' + e.riposoSec + 's</td>';
            corpo += '<td class="text-muted small">' + escapeHtml(e.note || "") + '</td>';
            corpo += '</tr>';
        }
        corpo += '</tbody></table>';
    }
    document.getElementById("modalCorpo").innerHTML = corpo;
    modalDettaglio.show();
}

// ----- Form crea/modifica -----

function apriFormNuovo() {
    document.getElementById("titoloFormProgramma").textContent = "Nuovo programma";
    document.getElementById("idProgramma").value = "";
    document.getElementById("immaginePrecedente").value = "";
    document.getElementById("nomeProgramma").value = "";
    document.getElementById("obiettivoProgramma").value = "";
    document.getElementById("difficoltaProgramma").value = "principiante";
    document.getElementById("durataProgramma").value = "4";
    document.getElementById("immagineProgramma").value = "";
    document.getElementById("anteprimaImmagine").classList.add("d-none");
    document.getElementById("righeEsercizi").innerHTML = "";
    aggiungiRiga();
    nascondiMessaggio();
    document.getElementById("cardFormProgramma").classList.remove("d-none");
    document.getElementById("cardFormProgramma").scrollIntoView({ behavior: "smooth" });
}

async function apriFormModifica(id) {
    const r = await apiGet("/api/programmi/" + id);
    if (!r.ok) { alert("Impossibile caricare il programma"); return; }

    const p = r.dati.programma;
    const esercizi = r.dati.esercizi;

    document.getElementById("titoloFormProgramma").textContent = "Modifica programma";
    document.getElementById("idProgramma").value = p.id;
    document.getElementById("immaginePrecedente").value = p.immagine || "";
    document.getElementById("nomeProgramma").value = p.nome;
    document.getElementById("obiettivoProgramma").value = p.obiettivo;
    document.getElementById("difficoltaProgramma").value = p.difficolta;
    document.getElementById("durataProgramma").value = p.durataSettimane;
    document.getElementById("immagineProgramma").value = "";

    if (p.immagine && p.immagine.length > 0) {
        document.getElementById("anteprimaImmagine").classList.remove("d-none");
        document.getElementById("imgAttuale").src = p.immagine;
    } else {
        document.getElementById("anteprimaImmagine").classList.add("d-none");
    }

    document.getElementById("righeEsercizi").innerHTML = "";
    for (const e of esercizi) aggiungiRiga(e);
    if (esercizi.length === 0) aggiungiRiga();
    nascondiMessaggio();
    document.getElementById("cardFormProgramma").classList.remove("d-none");
    document.getElementById("cardFormProgramma").scrollIntoView({ behavior: "smooth" });
}

function chiudiForm() {
    document.getElementById("cardFormProgramma").classList.add("d-none");
}

function aggiungiRiga(es) {
    const tbody = document.getElementById("righeEsercizi");
    const tr = document.createElement("tr");
    tr.innerHTML =
        '<td><input type="text" class="form-control form-control-sm" data-campo="nome" value="' + (es ? escapeAttr(es.nome) : "") + '" maxlength="100"></td>' +
        '<td><input type="number" class="form-control form-control-sm" data-campo="serie" min="1" max="20" value="' + (es ? es.serie : 3) + '"></td>' +
        '<td><input type="number" class="form-control form-control-sm" data-campo="ripetizioni" min="1" max="100" value="' + (es ? es.ripetizioni : 10) + '"></td>' +
        '<td><input type="number" class="form-control form-control-sm" data-campo="riposoSec" min="0" max="600" value="' + (es ? es.riposoSec : 60) + '"></td>' +
        '<td><input type="text" class="form-control form-control-sm" data-campo="note" value="' + (es ? escapeAttr(es.note || "") : "") + '" maxlength="300"></td>' +
        '<td><button type="button" class="btn btn-sm btn-outline-danger">-</button></td>';
    tbody.appendChild(tr);
    tr.querySelector("button").addEventListener("click", function () { tr.remove(); });
}

async function salvaProgramma(evento) {
    evento.preventDefault();

    // Upload immagine se l'utente ne ha selezionata una nuova.
    let urlImmagine = document.getElementById("immaginePrecedente").value;
    const fileInput = document.getElementById("immagineProgramma");
    if (fileInput.files && fileInput.files.length > 0) {
        const fd = new FormData();
        fd.append("file", fileInput.files[0]);
        const rispUpload = await fetch("/api/upload", {
            method: "POST",
            body: fd,
            credentials: "same-origin"
        });
        const datiUpload = await rispUpload.json();
        if (!rispUpload.ok) {
            mostraMessaggio("Errore upload immagine: " + (datiUpload.errore || ""), "danger");
            return;
        }
        urlImmagine = datiUpload.url;
    }

    const dati = {
        nome: document.getElementById("nomeProgramma").value.trim(),
        obiettivo: document.getElementById("obiettivoProgramma").value.trim(),
        difficolta: document.getElementById("difficoltaProgramma").value,
        durataSettimane: parseInt(document.getElementById("durataProgramma").value),
        immagine: urlImmagine,
        esercizi: leggiEsercizi()
    };

    const id = document.getElementById("idProgramma").value;
    const risposta = id
        ? await apiPut("/api/programmi/" + id, dati)
        : await apiPost("/api/programmi", dati);

    if (!risposta.ok) {
        mostraMessaggio(risposta.dati.errore || "Errore salvataggio", "danger");
        return;
    }
    chiudiForm();
    await caricaGriglia();
}

function leggiEsercizi() {
    const righe = document.querySelectorAll("#righeEsercizi tr");
    const esercizi = [];
    righe.forEach(function (tr) {
        const e = {
            nome: tr.querySelector('[data-campo="nome"]').value.trim(),
            serie: parseInt(tr.querySelector('[data-campo="serie"]').value) || 0,
            ripetizioni: parseInt(tr.querySelector('[data-campo="ripetizioni"]').value) || 0,
            riposoSec: parseInt(tr.querySelector('[data-campo="riposoSec"]').value) || 0,
            note: tr.querySelector('[data-campo="note"]').value.trim()
        };
        if (e.nome.length > 0) esercizi.push(e);
    });
    return esercizi;
}

async function eliminaProgramma(id) {
    if (!confirm("Eliminare definitivamente questo programma? Verranno cancellate anche le sessioni e i feedback dei clienti.")) return;
    const r = await apiDelete("/api/programmi/" + id);
    if (!r.ok) { alert(r.dati.errore || "Errore eliminazione"); return; }
    await caricaGriglia();
}

// ----- Helper messaggi form -----

function mostraMessaggio(testo, tipo) {
    const div = document.getElementById("messaggioFormProgramma");
    div.textContent = testo;
    div.className = "alert alert-" + tipo;
}

function nascondiMessaggio() {
    const div = document.getElementById("messaggioFormProgramma");
    div.textContent = "";
    div.className = "alert d-none";
}

function escapeHtml(s) {
    return String(s == null ? "" : s)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;");
}

function escapeAttr(s) {
    return escapeHtml(s);
}

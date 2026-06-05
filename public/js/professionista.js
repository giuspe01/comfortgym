// professionista.js - Dashboard professionista: gestione programmi e piani.

window.addEventListener("DOMContentLoaded", async function () {
    const utente = await richiediUtente("professionista");
    if (!utente) return;

    document.getElementById("nomeUtente").textContent = utente.nome;
    document.getElementById("specializzazione").textContent = utente.specializzazione || "";
    document.getElementById("btnLogout").addEventListener("click", logout);

    // Programmi
    document.getElementById("btnNuovoProgramma").addEventListener("click", apriFormNuovoProgramma);
    document.getElementById("btnAnnullaProgramma").addEventListener("click", chiudiFormProgramma);
    document.getElementById("btnAggiungiEsercizio").addEventListener("click", function () {
        aggiungiRigaEsercizio();
    });
    document.getElementById("formProgramma").addEventListener("submit", salvaProgramma);

    // Piani
    document.getElementById("btnNuovoPiano").addEventListener("click", apriFormNuovoPiano);
    document.getElementById("btnAnnullaPiano").addEventListener("click", chiudiFormPiano);
    document.getElementById("btnAggiungiPasto").addEventListener("click", function () {
        aggiungiRigaPasto();
    });
    document.getElementById("formPiano").addEventListener("submit", salvaPiano);

    await caricaProgrammi();
    await caricaPiani();
    await caricaAttivita();
    await caricaFeedback();
});

// =================== PROGRAMMI ===================

async function caricaProgrammi() {
    const cont = document.getElementById("elencoProgrammi");
    cont.innerHTML = '<p class="text-muted">Caricamento...</p>';

    const r = await apiGet("/api/professionista/programmi");
    if (!r.ok) {
        cont.innerHTML = '<p class="text-danger">Errore caricamento.</p>';
        return;
    }

    const programmi = r.dati.programmi;
    if (programmi.length === 0) {
        cont.innerHTML = '<p class="text-muted mb-0">Non hai ancora creato programmi.</p>';
        return;
    }

    let html = '<table class="table table-sm align-middle">';
    html += '<thead><tr><th>Nome</th><th>Obiettivo</th><th>Difficolta\'</th><th>Durata</th><th></th></tr></thead><tbody>';
    for (const p of programmi) {
        html += '<tr>';
        html += '<td>' + escapeHtml(p.nome) + '</td>';
        html += '<td>' + escapeHtml(p.obiettivo) + '</td>';
        html += '<td>' + escapeHtml(p.difficolta) + '</td>';
        html += '<td>' + p.durataSettimane + ' sett.</td>';
        html += '<td class="text-end">';
        html += '<button class="btn btn-sm btn-outline-primary me-1" data-modProg="' + p.id + '">Modifica</button>';
        html += '<button class="btn btn-sm btn-outline-danger" data-elimProg="' + p.id + '">Elimina</button>';
        html += '</td>';
        html += '</tr>';
    }
    html += '</tbody></table>';
    cont.innerHTML = html;

    cont.querySelectorAll("button[data-modProg]").forEach(function (btn) {
        btn.addEventListener("click", function () {
            apriFormModificaProgramma(parseInt(btn.getAttribute("data-modProg")));
        });
    });
    cont.querySelectorAll("button[data-elimProg]").forEach(function (btn) {
        btn.addEventListener("click", function () {
            eliminaProgramma(parseInt(btn.getAttribute("data-elimProg")));
        });
    });
}

function apriFormNuovoProgramma() {
    document.getElementById("titoloFormProgramma").textContent = "Nuovo programma";
    document.getElementById("idProgramma").value = "";
    document.getElementById("nomeProgramma").value = "";
    document.getElementById("obiettivoProgramma").value = "perdita_peso";
    document.getElementById("difficoltaProgramma").value = "principiante";
    document.getElementById("durataProgramma").value = "4";
    document.getElementById("righeEsercizi").innerHTML = "";
    aggiungiRigaEsercizio();
    nascondiMessaggioForm("messaggioFormProgramma");
    document.getElementById("cardFormProgramma").classList.remove("d-none");
    document.getElementById("cardFormProgramma").scrollIntoView({ behavior: "smooth" });
}

async function apriFormModificaProgramma(id) {
    const r = await apiGet("/api/programmi/" + id);
    if (!r.ok) { alert("Impossibile caricare il programma"); return; }
    const p = r.dati.programma;
    const esercizi = r.dati.esercizi;

    document.getElementById("titoloFormProgramma").textContent = "Modifica programma";
    document.getElementById("idProgramma").value = p.id;
    document.getElementById("nomeProgramma").value = p.nome;
    document.getElementById("obiettivoProgramma").value = p.obiettivo;
    document.getElementById("difficoltaProgramma").value = p.difficolta;
    document.getElementById("durataProgramma").value = p.durataSettimane;
    document.getElementById("righeEsercizi").innerHTML = "";
    for (const e of esercizi) aggiungiRigaEsercizio(e);
    if (esercizi.length === 0) aggiungiRigaEsercizio();
    nascondiMessaggioForm("messaggioFormProgramma");
    document.getElementById("cardFormProgramma").classList.remove("d-none");
    document.getElementById("cardFormProgramma").scrollIntoView({ behavior: "smooth" });
}

function chiudiFormProgramma() {
    document.getElementById("cardFormProgramma").classList.add("d-none");
}

function aggiungiRigaEsercizio(es) {
    const tbody = document.getElementById("righeEsercizi");
    const tr = document.createElement("tr");
    tr.innerHTML =
        '<td><input type="text" class="form-control form-control-sm" data-campo="nome" value="' + (es ? escapeAttr(es.nome) : "") + '" maxlength="100"></td>' +
        '<td><input type="number" class="form-control form-control-sm" data-campo="serie" min="1" max="20" value="' + (es ? es.serie : 3) + '"></td>' +
        '<td><input type="number" class="form-control form-control-sm" data-campo="ripetizioni" min="1" max="100" value="' + (es ? es.ripetizioni : 10) + '"></td>' +
        '<td><input type="number" class="form-control form-control-sm" data-campo="riposoSec" min="0" max="600" value="' + (es ? es.riposoSec : 60) + '"></td>' +
        '<td><input type="text" class="form-control form-control-sm" data-campo="note" value="' + (es ? escapeAttr(es.note) : "") + '" maxlength="300"></td>' +
        '<td><button type="button" class="btn btn-sm btn-outline-danger">-</button></td>';
    tbody.appendChild(tr);
    tr.querySelector("button").addEventListener("click", function () { tr.remove(); });
}

async function salvaProgramma(evento) {
    evento.preventDefault();
    const dati = {
        nome: document.getElementById("nomeProgramma").value.trim(),
        obiettivo: document.getElementById("obiettivoProgramma").value,
        difficolta: document.getElementById("difficoltaProgramma").value,
        durataSettimane: parseInt(document.getElementById("durataProgramma").value),
        esercizi: leggiEserciziDalForm()
    };

    const id = document.getElementById("idProgramma").value;
    const risposta = id
        ? await apiPut("/api/programmi/" + id, dati)
        : await apiPost("/api/programmi", dati);

    if (!risposta.ok) {
        mostraMessaggioForm("messaggioFormProgramma", risposta.dati.errore || "Errore", "danger");
        return;
    }
    chiudiFormProgramma();
    await caricaProgrammi();
}

function leggiEserciziDalForm() {
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
    if (!confirm("Eliminare definitivamente questo programma?")) return;
    const r = await apiDelete("/api/programmi/" + id);
    if (!r.ok) { alert(r.dati.errore || "Errore eliminazione"); return; }
    await caricaProgrammi();
}

// =================== PIANI ===================

async function caricaPiani() {
    const cont = document.getElementById("elencoPiani");
    cont.innerHTML = '<p class="text-muted">Caricamento...</p>';

    const r = await apiGet("/api/professionista/piani");
    if (!r.ok) {
        cont.innerHTML = '<p class="text-danger">Errore caricamento.</p>';
        return;
    }

    const piani = r.dati.piani;
    if (piani.length === 0) {
        cont.innerHTML = '<p class="text-muted mb-0">Non hai ancora creato piani.</p>';
        return;
    }

    let html = '<table class="table table-sm align-middle">';
    html += '<thead><tr><th>Nome</th><th>Calorie giornaliere</th><th></th></tr></thead><tbody>';
    for (const p of piani) {
        html += '<tr>';
        html += '<td>' + escapeHtml(p.nome) + '</td>';
        html += '<td>' + p.calorieGiornaliere + '</td>';
        html += '<td class="text-end">';
        html += '<button class="btn btn-sm btn-outline-primary me-1" data-modPiano="' + p.id + '">Modifica</button>';
        html += '<button class="btn btn-sm btn-outline-danger" data-elimPiano="' + p.id + '">Elimina</button>';
        html += '</td>';
        html += '</tr>';
    }
    html += '</tbody></table>';
    cont.innerHTML = html;

    cont.querySelectorAll("button[data-modPiano]").forEach(function (btn) {
        btn.addEventListener("click", function () {
            apriFormModificaPiano(parseInt(btn.getAttribute("data-modPiano")));
        });
    });
    cont.querySelectorAll("button[data-elimPiano]").forEach(function (btn) {
        btn.addEventListener("click", function () {
            eliminaPiano(parseInt(btn.getAttribute("data-elimPiano")));
        });
    });
}

function apriFormNuovoPiano() {
    document.getElementById("titoloFormPiano").textContent = "Nuovo piano";
    document.getElementById("idPiano").value = "";
    document.getElementById("nomePiano").value = "";
    document.getElementById("caloriePiano").value = "2000";
    document.getElementById("righePasti").innerHTML = "";
    aggiungiRigaPasto();
    nascondiMessaggioForm("messaggioFormPiano");
    document.getElementById("cardFormPiano").classList.remove("d-none");
    document.getElementById("cardFormPiano").scrollIntoView({ behavior: "smooth" });
}

async function apriFormModificaPiano(id) {
    const r = await apiGet("/api/piani/" + id);
    if (!r.ok) { alert("Impossibile caricare il piano"); return; }
    const p = r.dati.piano;
    const pasti = r.dati.pasti;

    document.getElementById("titoloFormPiano").textContent = "Modifica piano";
    document.getElementById("idPiano").value = p.id;
    document.getElementById("nomePiano").value = p.nome;
    document.getElementById("caloriePiano").value = p.calorieGiornaliere;
    document.getElementById("righePasti").innerHTML = "";
    for (const pa of pasti) aggiungiRigaPasto(pa);
    if (pasti.length === 0) aggiungiRigaPasto();
    nascondiMessaggioForm("messaggioFormPiano");
    document.getElementById("cardFormPiano").classList.remove("d-none");
    document.getElementById("cardFormPiano").scrollIntoView({ behavior: "smooth" });
}

function chiudiFormPiano() {
    document.getElementById("cardFormPiano").classList.add("d-none");
}

function aggiungiRigaPasto(pa) {
    const tbody = document.getElementById("righePasti");
    const tr = document.createElement("tr");
    const tipo = pa ? pa.tipo : "colazione";
    tr.innerHTML =
        '<td><select class="form-select form-select-sm" data-campo="tipo">' +
        '<option value="colazione"' + (tipo === "colazione" ? " selected" : "") + '>Colazione</option>' +
        '<option value="pranzo"' + (tipo === "pranzo" ? " selected" : "") + '>Pranzo</option>' +
        '<option value="cena"' + (tipo === "cena" ? " selected" : "") + '>Cena</option>' +
        '<option value="spuntino"' + (tipo === "spuntino" ? " selected" : "") + '>Spuntino</option>' +
        '</select></td>' +
        '<td><input type="text" class="form-control form-control-sm" data-campo="descrizione" value="' + (pa ? escapeAttr(pa.descrizione) : "") + '" maxlength="300"></td>' +
        '<td><input type="number" class="form-control form-control-sm" data-campo="calorie" min="0" max="3000" value="' + (pa ? pa.calorie : 400) + '"></td>' +
        '<td><button type="button" class="btn btn-sm btn-outline-danger">-</button></td>';
    tbody.appendChild(tr);
    tr.querySelector("button").addEventListener("click", function () { tr.remove(); });
}

async function salvaPiano(evento) {
    evento.preventDefault();
    const dati = {
        nome: document.getElementById("nomePiano").value.trim(),
        calorieGiornaliere: parseInt(document.getElementById("caloriePiano").value),
        pasti: leggiPastiDalForm()
    };

    const id = document.getElementById("idPiano").value;
    const risposta = id
        ? await apiPut("/api/piani/" + id, dati)
        : await apiPost("/api/piani", dati);

    if (!risposta.ok) {
        mostraMessaggioForm("messaggioFormPiano", risposta.dati.errore || "Errore", "danger");
        return;
    }
    chiudiFormPiano();
    await caricaPiani();
}

function leggiPastiDalForm() {
    const righe = document.querySelectorAll("#righePasti tr");
    const pasti = [];
    righe.forEach(function (tr) {
        const p = {
            tipo: tr.querySelector('[data-campo="tipo"]').value,
            descrizione: tr.querySelector('[data-campo="descrizione"]').value.trim(),
            calorie: parseInt(tr.querySelector('[data-campo="calorie"]').value) || 0
        };
        if (p.descrizione.length > 0) pasti.push(p);
    });
    return pasti;
}

async function eliminaPiano(id) {
    if (!confirm("Eliminare definitivamente questo piano?")) return;
    const r = await apiDelete("/api/piani/" + id);
    if (!r.ok) { alert(r.dati.errore || "Errore eliminazione"); return; }
    await caricaPiani();
}

// =================== ATTIVITA' ===================

async function caricaAttivita() {
    const cont = document.getElementById("elencoAttivita");
    cont.innerHTML = '<p class="text-muted">Caricamento...</p>';

    const r = await apiGet("/api/professionista/attivita");
    if (!r.ok) {
        cont.innerHTML = '<p class="text-danger">Errore caricamento.</p>';
        return;
    }

    const righe = r.dati.attivita;
    if (righe.length === 0) {
        cont.innerHTML = '<p class="text-muted mb-0">Nessun programma creato.</p>';
        return;
    }

    let html = '<table class="table table-sm align-middle">';
    html += '<thead><tr><th>Programma</th><th class="text-end">Sessioni totali</th><th class="text-end">Completate</th><th class="text-end">Adesione</th><th>Ultima sessione</th></tr></thead><tbody>';
    for (const r of righe) {
        const adesione = (r.totale > 0)
            ? Math.round((r.completate / r.totale) * 100) + "%"
            : "-";
        html += '<tr>';
        html += '<td>' + escapeHtml(r.nomeProgramma) + '</td>';
        html += '<td class="text-end">' + r.totale + '</td>';
        html += '<td class="text-end">' + r.completate + '</td>';
        html += '<td class="text-end">' + adesione + '</td>';
        html += '<td>' + escapeHtml(r.ultimaData || "—") + '</td>';
        html += '</tr>';
    }
    html += '</tbody></table>';
    cont.innerHTML = html;
}

// =================== FEEDBACK ===================

async function caricaFeedback() {
    const cont = document.getElementById("elencoFeedback");
    cont.innerHTML = '<p class="text-muted">Caricamento...</p>';

    const r = await apiGet("/api/professionista/feedback");
    if (!r.ok) {
        cont.innerHTML = '<p class="text-danger">Errore caricamento.</p>';
        return;
    }

    const elenco = r.dati.feedback;
    if (elenco.length === 0) {
        cont.innerHTML = '<p class="text-muted mb-0">Nessun feedback ricevuto.</p>';
        return;
    }

    let html = '<table class="table table-sm align-middle">';
    html += '<thead><tr><th>Programma</th><th>Cliente</th><th>Voto</th><th>Commento</th><th>Data</th></tr></thead><tbody>';
    for (const f of elenco) {
        html += '<tr>';
        html += '<td>' + escapeHtml(f.nomeProgramma) + '</td>';
        html += '<td>' + escapeHtml(f.nomeCliente) + '</td>';
        html += '<td>' + stelle(f.voto) + '</td>';
        html += '<td>' + escapeHtml(f.commento || "") + '</td>';
        html += '<td>' + escapeHtml(f.data) + '</td>';
        html += '</tr>';
    }
    html += '</tbody></table>';
    cont.innerHTML = html;
}

function stelle(voto) {
    let s = "";
    for (let i = 1; i <= 5; i++) s += (i <= voto) ? "★" : "☆";
    return s;
}

// ----- Helper messaggi e escaping -----

function mostraMessaggioForm(idDiv, testo, tipo) {
    const div = document.getElementById(idDiv);
    div.textContent = testo;
    div.className = "alert alert-" + tipo;
}

function nascondiMessaggioForm(idDiv) {
    const div = document.getElementById(idDiv);
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

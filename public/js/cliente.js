// cliente.js - Dashboard cliente: programma corrente, piano nutrizionale, sessioni, aderenza.

let utenteCorrente = null;
let modalSessione = null;
let modalAderenza = null;

window.addEventListener("DOMContentLoaded", async function () {
    utenteCorrente = await richiediUtente("cliente");
    if (!utenteCorrente) return;

    document.getElementById("nomeUtente").textContent = utenteCorrente.nome;
    document.getElementById("btnLogout").addEventListener("click", logout);

    modalSessione = new bootstrap.Modal(document.getElementById("modalDettaglioSessione"));
    modalAderenza = new bootstrap.Modal(document.getElementById("modalDettaglioAderenza"));

    document.getElementById("btnNuovaSessione").addEventListener("click", apriFormSessione);
    document.getElementById("btnAnnullaSessione").addEventListener("click", chiudiFormSessione);
    document.getElementById("formSessione").addEventListener("submit", salvaSessione);

    document.getElementById("btnNuovaAderenza").addEventListener("click", apriFormAderenza);
    document.getElementById("btnAnnullaAderenza").addEventListener("click", chiudiFormAderenza);
    document.getElementById("formAderenza").addEventListener("submit", salvaAderenza);
    document.getElementById("aderenzaPercentuale").addEventListener("input", function () {
        document.getElementById("aderenzaValoreLabel").textContent = this.value;
    });

    mostraInfoUtente();
    await caricaProgrammaCorrente();
    await caricaPianoCorrente();
    await caricaSessioni();
    await caricaAderenze();
});

// ----- Info utente -----

function mostraInfoUtente() {
    const u = utenteCorrente;
    const cont = document.getElementById("infoUtente");

    const obiettivoLeggibile = {
        perdita_peso: "Perdita peso",
        massa: "Aumento massa",
        mantenimento: "Mantenimento"
    };

    let html = '<div class="row g-2 mb-2">';
    html += '<div class="col-sm-6 col-md-4"><span class="text-muted small">Nome</span><div>' + escapeHtml(u.nome) + ' ' + escapeHtml(u.cognome) + '</div></div>';
    html += '<div class="col-sm-6 col-md-4"><span class="text-muted small">Username</span><div>' + escapeHtml(u.username) + '</div></div>';
    html += '<div class="col-sm-6 col-md-4"><span class="text-muted small">Email</span><div>' + escapeHtml(u.email) + '</div></div>';
    html += '<div class="col-sm-6 col-md-4"><span class="text-muted small">Eta\'</span><div>' + (u.eta || '—') + ' anni</div></div>';
    html += '<div class="col-sm-6 col-md-4"><span class="text-muted small">Peso</span><div>' + (u.peso || '—') + ' kg</div></div>';
    html += '<div class="col-sm-6 col-md-4"><span class="text-muted small">Altezza</span><div>' + (u.altezza || '—') + ' cm</div></div>';
    html += '<div class="col-sm-6 col-md-4"><span class="text-muted small">Obiettivo</span><div>' + escapeHtml(obiettivoLeggibile[u.obiettivo] || u.obiettivo || '—') + '</div></div>';
    html += '<div class="col-sm-6 col-md-4"><span class="text-muted small">Iscritto dal</span><div>' + escapeHtml(u.dataRegistrazione) + '</div></div>';
    html += '</div>';
    html += '<button id="btnModificaProfilo" class="btn btn-sm btn-outline-primary">Modifica profilo</button>';
    html += '<div id="formModificaProfilo" class="d-none mt-3 border rounded p-3">';
    html += '<div class="row g-2 mb-2">';
    html += '<div class="col-sm-4"><label class="form-label small mb-1">Eta\'</label>';
    html += '<input type="number" id="editEta" class="form-control form-control-sm" min="14" max="100" value="' + (u.eta || 0) + '"></div>';
    html += '<div class="col-sm-4"><label class="form-label small mb-1">Peso (kg)</label>';
    html += '<input type="number" id="editPeso" class="form-control form-control-sm" min="30" max="250" step="0.1" value="' + (u.peso || 0) + '"></div>';
    html += '<div class="col-sm-4"><label class="form-label small mb-1">Altezza (cm)</label>';
    html += '<input type="number" id="editAltezza" class="form-control form-control-sm" min="120" max="230" step="0.1" value="' + (u.altezza || 0) + '"></div>';
    html += '</div>';
    html += '<p id="erroreModificaProfilo" class="text-danger small mb-2"></p>';
    html += '<button id="btnSalvaProfilo" class="btn btn-sm btn-primary me-1">Salva</button>';
    html += '<button id="btnAnnullaProfilo" class="btn btn-sm btn-outline-secondary">Annulla</button>';
    html += '</div>';
    cont.innerHTML = html;

    document.getElementById("btnModificaProfilo").addEventListener("click", function () {
        document.getElementById("formModificaProfilo").classList.toggle("d-none");
    });

    document.getElementById("btnAnnullaProfilo").addEventListener("click", function () {
        document.getElementById("formModificaProfilo").classList.add("d-none");
    });

    document.getElementById("btnSalvaProfilo").addEventListener("click", async function () {
        const dati = {
            eta: parseInt(document.getElementById("editEta").value),
            peso: parseFloat(document.getElementById("editPeso").value),
            altezza: parseFloat(document.getElementById("editAltezza").value)
        };
        const r = await apiPut("/api/cliente/profilo", dati);
        if (!r.ok) {
            document.getElementById("erroreModificaProfilo").textContent = r.dati.errore || "Errore";
            return;
        }
        utenteCorrente = r.dati.utente;
        mostraInfoUtente();
    });
}

// ----- Programma corrente -----

async function caricaProgrammaCorrente() {
    const cont = document.getElementById("contenutoProgrammaCorrente");
    const id = utenteCorrente.idProgrammaCorrente;
    if (!id) {
        cont.innerHTML = '<p class="text-muted mb-0">Nessun programma selezionato. <a href="catalogo-programmi.html">Scegli un programma dal catalogo.</a></p>';
        return;
    }

    const r = await apiGet("/api/programmi/" + id);
    if (!r.ok) {
        cont.innerHTML = '<p class="text-danger mb-0">Programma non trovato. <a href="catalogo-programmi.html">Scegli un nuovo programma.</a></p>';
        return;
    }

    const p = r.dati.programma;
    const esercizi = r.dati.esercizi;

    let html = '<h3 class="h6 mb-1">' + escapeHtml(p.nome) + '</h3>';
    html += '<p class="small text-muted mb-2">';
    html += 'Obiettivo: ' + escapeHtml(p.obiettivo) + ' | ';
    html += 'Difficolta\': ' + escapeHtml(p.difficolta) + ' | ';
    html += 'Durata: ' + p.durataSettimane + ' settimane';
    html += '</p>';

    if (esercizi.length === 0) {
        html += '<p class="text-muted">Nessun esercizio.</p>';
    } else {
        html += '<table class="table table-sm">';
        html += '<thead><tr><th>Esercizio</th><th>Serie</th><th>Ripetizioni</th><th>Riposo (sec)</th><th>Note</th></tr></thead><tbody>';
        for (const e of esercizi) {
            html += '<tr>';
            html += '<td>' + escapeHtml(e.nome) + '</td>';
            html += '<td>' + e.serie + '</td>';
            html += '<td>' + e.ripetizioni + '</td>';
            html += '<td>' + e.riposoSec + '</td>';
            html += '<td>' + escapeHtml(e.note || "") + '</td>';
            html += '</tr>';
        }
        html += '</tbody></table>';
    }

    html += '<div id="zonaFeedback" class="mt-3"></div>';
    html += '<button id="btnRimuoviProgramma" class="btn btn-sm btn-outline-danger">Rimuovi selezione</button>';
    cont.innerHTML = html;

    document.getElementById("btnRimuoviProgramma").addEventListener("click", async function () {
        await apiPost("/api/cliente/programma", { idProgramma: 0 });
        utenteCorrente.idProgrammaCorrente = 0;
        programmaCorrenteNome = "";
        await caricaProgrammaCorrente();
    });

    programmaCorrenteNome = p.nome;

    await caricaFeedbackProgramma(id);
}

// Nome del programma corrente, usato nel form sessione.
let programmaCorrenteNome = "";

async function caricaFeedbackProgramma(idProgramma) {
    const zona = document.getElementById("zonaFeedback");
    if (!zona) return;

    const r = await apiGet("/api/feedback/programma/" + idProgramma);
    if (!r.ok) { zona.innerHTML = ""; return; }

    const f = r.dati.feedback;
    if (f) {
        let html = '<div class="border rounded p-2 bg-light">';
        html += '<p class="small text-muted mb-1">Hai gia\' lasciato un feedback il ' + escapeHtml(f.data) + ':</p>';
        html += '<p class="mb-1"><strong>Voto:</strong> ' + stelle(f.voto) + '</p>';
        if (f.commento) {
            html += '<p class="mb-2"><strong>Commento:</strong> ' + escapeHtml(f.commento) + '</p>';
        }
        html += '<button id="btnModFeedback" class="btn btn-sm btn-outline-primary">Modifica</button>';
        html += '</div>';
        zona.innerHTML = html;
        document.getElementById("btnModFeedback").addEventListener("click", function () {
            mostraFormFeedback(idProgramma, f);
        });
    } else {
        zona.innerHTML = '<button id="btnLasciaFeedback" class="btn btn-sm btn-outline-primary">Lascia feedback</button>';
        document.getElementById("btnLasciaFeedback").addEventListener("click", function () {
            mostraFormFeedback(idProgramma, null);
        });
    }
}

function stelle(voto) {
    let s = "";
    for (let i = 1; i <= 5; i++) {
        s += (i <= voto) ? "★" : "☆";
    }
    return s + " (" + voto + "/5)";
}

function mostraFormFeedback(idProgramma, esistente) {
    const zona = document.getElementById("zonaFeedback");
    let html = '<form id="formFeedback" class="border rounded p-2 bg-light">';
    html += '<div class="mb-2">';
    html += '<label class="form-label small mb-1">Voto</label>';
    html += '<select id="fbVoto" class="form-select form-select-sm" style="max-width: 200px">';
    for (let i = 1; i <= 5; i++) {
        const sel = (esistente && esistente.voto === i) ? " selected" : "";
        html += '<option value="' + i + '"' + sel + '>' + stelle(i) + '</option>';
    }
    html += '</select></div>';
    html += '<div class="mb-2">';
    html += '<label class="form-label small mb-1">Commento (opzionale)</label>';
    html += '<textarea id="fbCommento" class="form-control form-control-sm" maxlength="500" rows="2">' + (esistente ? escapeHtml(esistente.commento) : "") + '</textarea>';
    html += '</div>';
    html += '<button type="submit" class="btn btn-sm btn-primary">Invia</button>';
    html += '<button type="button" id="btnAnnullaFb" class="btn btn-sm btn-outline-secondary ms-1">Annulla</button>';
    html += '<p id="fbErrore" class="text-danger small mt-2 mb-0"></p>';
    html += '</form>';
    zona.innerHTML = html;

    document.getElementById("btnAnnullaFb").addEventListener("click", function () {
        caricaFeedbackProgramma(idProgramma);
    });
    document.getElementById("formFeedback").addEventListener("submit", async function (e) {
        e.preventDefault();
        const dati = {
            idProgramma: idProgramma,
            voto: parseInt(document.getElementById("fbVoto").value),
            commento: document.getElementById("fbCommento").value.trim()
        };
        const r = await apiPost("/api/feedback", dati);
        if (!r.ok) {
            document.getElementById("fbErrore").textContent = r.dati.errore || "Errore";
            return;
        }
        await caricaFeedbackProgramma(idProgramma);
    });
}

// ----- Piano nutrizionale corrente -----

async function caricaPianoCorrente() {
    const cont = document.getElementById("contenutoPianoCorrente");
    const id = utenteCorrente.idPianoCorrente;
    if (!id) {
        cont.innerHTML = '<p class="text-muted mb-0">Nessun piano selezionato. <a href="catalogo-piani.html">Scegli un piano dal catalogo.</a></p>';
        return;
    }

    const r = await apiGet("/api/piani/" + id);
    if (!r.ok) {
        cont.innerHTML = '<p class="text-danger mb-0">Piano non trovato. <a href="catalogo-piani.html">Scegli un nuovo piano.</a></p>';
        return;
    }

    const p = r.dati.piano;
    const pasti = r.dati.pasti;

    let html = '<h3 class="h6 mb-1">' + escapeHtml(p.nome) + '</h3>';
    html += '<p class="small text-muted mb-2">Calorie giornaliere: ' + p.calorieGiornaliere + '</p>';

    if (p.consigliGiornalieri && p.consigliGiornalieri.trim().length > 0) {
        html += '<div class="alert alert-warning py-2 small mb-2">';
        html += '<strong>Consigli del nutrizionista:</strong> ' + escapeHtml(p.consigliGiornalieri);
        html += '</div>';
    }

    if (pasti.length === 0) {
        html += '<p class="text-muted">Nessun pasto.</p>';
    } else {
        html += '<table class="table table-sm">';
        html += '<thead><tr><th>Pasto</th><th>Descrizione</th><th>Calorie</th></tr></thead><tbody>';
        for (const pa of pasti) {
            html += '<tr>';
            html += '<td class="text-capitalize">' + escapeHtml(pa.tipo) + '</td>';
            html += '<td>' + escapeHtml(pa.descrizione) + '</td>';
            html += '<td>' + pa.calorie + '</td>';
            html += '</tr>';
        }
        html += '</tbody></table>';
    }

    html += '<div id="zonaFeedbackPiano" class="mt-3"></div>';
    html += '<button id="btnRimuoviPiano" class="btn btn-sm btn-outline-danger">Rimuovi selezione</button>';
    cont.innerHTML = html;

    document.getElementById("btnRimuoviPiano").addEventListener("click", async function () {
        await apiPost("/api/cliente/piano", { idPiano: 0 });
        utenteCorrente.idPianoCorrente = 0;
        await caricaPianoCorrente();
    });

    await caricaFeedbackPiano(id);
}

async function caricaFeedbackPiano(idPiano) {
    const zona = document.getElementById("zonaFeedbackPiano");
    if (!zona) return;

    const r = await apiGet("/api/feedback-piani/piano/" + idPiano);
    if (!r.ok) { zona.innerHTML = ""; return; }

    const f = r.dati.feedback;
    if (f) {
        let html = '<div class="border rounded p-2 bg-light mb-2">';
        html += '<p class="small text-muted mb-1">Hai gia\' lasciato un feedback il ' + escapeHtml(f.data) + ':</p>';
        html += '<p class="mb-1"><strong>Voto:</strong> ' + stelle(f.voto) + '</p>';
        if (f.commento) {
            html += '<p class="mb-2"><strong>Commento:</strong> ' + escapeHtml(f.commento) + '</p>';
        }
        html += '<button id="btnModFeedbackPiano" class="btn btn-sm btn-outline-primary">Modifica</button>';
        html += '</div>';
        zona.innerHTML = html;
        document.getElementById("btnModFeedbackPiano").addEventListener("click", function () {
            mostraFormFeedbackPiano(idPiano, f);
        });
    } else {
        zona.innerHTML = '<button id="btnLasciaFeedbackPiano" class="btn btn-sm btn-outline-primary mb-2">Lascia feedback sul piano</button>';
        document.getElementById("btnLasciaFeedbackPiano").addEventListener("click", function () {
            mostraFormFeedbackPiano(idPiano, null);
        });
    }
}

function mostraFormFeedbackPiano(idPiano, esistente) {
    const zona = document.getElementById("zonaFeedbackPiano");
    let html = '<form id="formFeedbackPiano" class="border rounded p-2 bg-light mb-2">';
    html += '<div class="mb-2">';
    html += '<label class="form-label small mb-1">Voto</label>';
    html += '<select id="fbpVoto" class="form-select form-select-sm" style="max-width: 200px">';
    for (let i = 1; i <= 5; i++) {
        const sel = (esistente && esistente.voto === i) ? " selected" : "";
        html += '<option value="' + i + '"' + sel + '>' + stelle(i) + '</option>';
    }
    html += '</select></div>';
    html += '<div class="mb-2">';
    html += '<label class="form-label small mb-1">Commento (opzionale)</label>';
    html += '<textarea id="fbpCommento" class="form-control form-control-sm" maxlength="500" rows="2">' + (esistente ? escapeHtml(esistente.commento) : "") + '</textarea>';
    html += '</div>';
    html += '<button type="submit" class="btn btn-sm btn-primary">Invia</button>';
    html += '<button type="button" id="btnAnnullaFbp" class="btn btn-sm btn-outline-secondary ms-1">Annulla</button>';
    html += '<p id="fbpErrore" class="text-danger small mt-2 mb-0"></p>';
    html += '</form>';
    zona.innerHTML = html;

    document.getElementById("btnAnnullaFbp").addEventListener("click", function () {
        caricaFeedbackPiano(idPiano);
    });
    document.getElementById("formFeedbackPiano").addEventListener("submit", async function (e) {
        e.preventDefault();
        const dati = {
            idPiano: idPiano,
            voto: parseInt(document.getElementById("fbpVoto").value),
            commento: document.getElementById("fbpCommento").value.trim()
        };
        const r = await apiPost("/api/feedback-piani", dati);
        if (!r.ok) {
            document.getElementById("fbpErrore").textContent = r.dati.errore || "Errore";
            return;
        }
        await caricaFeedbackPiano(idPiano);
    });
}

// ----- Sessioni -----

async function caricaSessioni() {
    const cont = document.getElementById("elencoSessioni");
    cont.innerHTML = '<p class="text-muted">Caricamento...</p>';

    const r = await apiGet("/api/sessioni/me");
    if (!r.ok) {
        cont.innerHTML = '<p class="text-danger">Errore caricamento sessioni.</p>';
        return;
    }

    const sessioni = r.dati.sessioni;
    if (sessioni.length === 0) {
        cont.innerHTML = '<p class="text-muted mb-0">Non hai ancora registrato sessioni.</p>';
        return;
    }

    const totali = sessioni.length;
    const completate = sessioni.filter(function (s) { return s.completata; }).length;
    const adesione = Math.round((completate / totali) * 100);

    let html = '<div class="alert alert-info py-2 mb-3">';
    html += '<strong>Adesione al programma:</strong> hai svolto tutti gli esercizi previsti in ';
    html += '<strong>' + completate + ' su ' + totali + '</strong> delle sessioni registrate ';
    html += '(' + adesione + '%).';
    html += '</div>';
    html += '<table class="table table-sm align-middle">';
    html += '<thead><tr><th>Data</th><th>Programma</th><th>Durata</th><th>Stato</th><th>Note</th><th></th></tr></thead><tbody>';
    for (const s of sessioni) {
        html += '<tr>';
        html += '<td>' + escapeHtml(s.data) + '</td>';
        html += '<td class="text-muted small">' + escapeHtml(s.nomeProgramma || "") + '</td>';
        html += '<td>' + s.durataMin + ' min</td>';
        if (s.completata) {
            html += '<td><span class="badge bg-success">Completata</span></td>';
        } else {
            html += '<td><span class="badge bg-warning text-dark">Interrotta</span></td>';
        }
        html += '<td>' + escapeHtml(s.note || "") + '</td>';
        html += '<td class="text-end">';
        html += '<button class="btn btn-sm btn-outline-secondary me-1" data-dettaglioSess="' + s.id + '">Dettaglio</button>';
        html += '<button class="btn btn-sm btn-outline-danger" data-elimSess="' + s.id + '">Elimina</button>';
        html += '</td>';
        html += '</tr>';
    }
    html += '</tbody></table>';
    cont.innerHTML = html;

    // Mappa id -> oggetto sessione per aprire il dettaglio senza rifare la fetch.
    const mappaSessioni = {};
    for (const s of sessioni) mappaSessioni[s.id] = s;

    cont.querySelectorAll("button[data-dettaglioSess]").forEach(function (btn) {
        btn.addEventListener("click", function () {
            apriDettaglioSessione(mappaSessioni[parseInt(btn.getAttribute("data-dettaglioSess"))]);
        });
    });

    cont.querySelectorAll("button[data-elimSess]").forEach(function (btn) {
        btn.addEventListener("click", async function () {
            if (!confirm("Eliminare questa sessione?")) return;
            const id = parseInt(btn.getAttribute("data-elimSess"));
            await apiDelete("/api/sessioni/" + id);
            await caricaSessioni();
        });
    });
}

async function apriDettaglioSessione(s) {
    // ---- Dati sessione ----
    let html = '<h6 class="text-muted mb-2">Sessione</h6>';
    html += '<dl class="row mb-3">';
    html += '<dt class="col-5">Data</dt><dd class="col-7">' + escapeHtml(s.data) + '</dd>';
    html += '<dt class="col-5">Programma</dt><dd class="col-7">' + escapeHtml(s.nomeProgramma || "—") + '</dd>';
    html += '<dt class="col-5">Durata</dt><dd class="col-7">' + s.durataMin + ' minuti</dd>';
    html += '<dt class="col-5">Stato</dt><dd class="col-7">';
    html += s.completata
        ? '<span class="badge bg-success">Completata</span>'
        : '<span class="badge bg-warning text-dark">Interrotta</span>';
    html += '</dd>';
    html += '<dt class="col-5">Note</dt><dd class="col-7">' + escapeHtml(s.note || "—") + '</dd>';
    html += '</dl>';

    // ---- Esercizi del programma ----
    html += '<h6 class="text-muted mb-2">Esercizi del programma</h6>';
    if (s.idProgramma) {
        const r = await apiGet("/api/programmi/" + s.idProgramma);
        if (!r.ok || r.dati.esercizi.length === 0) {
            html += '<p class="text-muted small">Nessun esercizio definito nel programma.</p>';
        } else {
            html += '<table class="table table-sm">';
            html += '<thead><tr><th>Esercizio</th><th>Serie</th><th>Rip.</th><th>Riposo</th><th>Note</th></tr></thead><tbody>';
            for (const e of r.dati.esercizi) {
                html += '<tr>';
                html += '<td>' + escapeHtml(e.nome) + '</td>';
                html += '<td>' + e.serie + '</td>';
                html += '<td>' + e.ripetizioni + '</td>';
                html += '<td>' + e.riposoSec + 's</td>';
                html += '<td class="text-muted small">' + escapeHtml(e.note || "") + '</td>';
                html += '</tr>';
            }
            html += '</tbody></table>';
        }
    } else {
        html += '<p class="text-muted small">Programma non disponibile.</p>';
    }

    document.getElementById("modalCorpoSessione").innerHTML = html;
    modalSessione.show();
}

async function apriFormSessione() {
    if (!utenteCorrente.idProgrammaCorrente) {
        alert("Seleziona prima un programma dal catalogo.");
        return;
    }

    const r = await apiGet("/api/programmi/" + utenteCorrente.idProgrammaCorrente);
    if (!r.ok) {
        alert("Impossibile caricare il programma");
        return;
    }
    const esercizi = r.dati.esercizi;

    document.getElementById("sessioneNomeProgramma").textContent = programmaCorrenteNome;

    const oggi = new Date().toISOString().split("T")[0];
    document.getElementById("sessioneData").value = oggi;
    document.getElementById("sessioneDurata").value = 45;
    document.getElementById("sessioneNote").value = "";
    document.getElementById("messaggioSessione").textContent = "";

    const cont = document.getElementById("sessioneEserciziSvolti");
    if (esercizi.length === 0) {
        cont.innerHTML = '<p class="text-muted small mb-0">Questo programma non ha esercizi: la sessione si conta automaticamente come completata.</p>';
    } else {
        let html = "";
        for (const e of esercizi) {
            html += '<div class="form-check">';
            html += '<input class="form-check-input sessione-esercizio" type="checkbox" id="es' + e.id + '" checked>';
            html += '<label class="form-check-label small" for="es' + e.id + '">';
            html += escapeHtml(e.nome) + ' <span class="text-muted">(' + e.serie + 'x' + e.ripetizioni + ')</span>';
            html += '</label></div>';
        }
        cont.innerHTML = html;
    }

    document.getElementById("cardFormSessione").classList.remove("d-none");
}

function chiudiFormSessione() {
    document.getElementById("cardFormSessione").classList.add("d-none");
}

async function salvaSessione(evento) {
    evento.preventDefault();

    const tutte = document.querySelectorAll(".sessione-esercizio");
    const svolte = document.querySelectorAll(".sessione-esercizio:checked");
    const eserciziTotali = tutte.length;
    const eserciziSvolti = svolte.length;
    const completata = (eserciziTotali === 0) || (eserciziSvolti === eserciziTotali);

    const noteOriginali = document.getElementById("sessioneNote").value.trim();
    let noteFinali = noteOriginali;
    if (eserciziTotali > 0) {
        const prefisso = "Esercizi svolti: " + eserciziSvolti + "/" + eserciziTotali + ".";
        noteFinali = noteOriginali ? (prefisso + " " + noteOriginali) : prefisso;
    }

    const dati = {
        idProgramma: utenteCorrente.idProgrammaCorrente,
        data: document.getElementById("sessioneData").value,
        durataMin: parseInt(document.getElementById("sessioneDurata").value),
        completata: completata,
        note: noteFinali
    };
    const r = await apiPost("/api/sessioni", dati);
    if (!r.ok) {
        document.getElementById("messaggioSessione").textContent = r.dati.errore || "Errore";
        return;
    }
    chiudiFormSessione();
    await caricaSessioni();
}

// ----- Aderenza al piano nutrizionale -----

async function caricaAderenze() {
    const cont = document.getElementById("elencoAderenze");
    cont.innerHTML = '<p class="text-muted">Caricamento...</p>';

    const r = await apiGet("/api/aderenza/me");
    if (!r.ok) {
        cont.innerHTML = '<p class="text-danger">Errore caricamento aderenze.</p>';
        return;
    }

    const lista = r.dati.aderenze;
    if (lista.length === 0) {
        cont.innerHTML = '<p class="text-muted mb-0">Non hai ancora registrato giornate di aderenza.</p>';
        return;
    }

    const ultime7 = lista.slice(0, 7);
    const mediaUltime = Math.round(
        ultime7.reduce(function (acc, a) { return acc + a.percentuale; }, 0) / ultime7.length
    );
    const mediaTotale = Math.round(
        lista.reduce(function (acc, a) { return acc + a.percentuale; }, 0) / lista.length
    );

    let html = '<div class="alert alert-success py-2 mb-3 small">';
    html += '<strong>Aderenza media:</strong> ' + mediaTotale + '% complessiva';
    if (lista.length >= 7) {
        html += ', ' + mediaUltime + '% negli ultimi 7 giorni';
    }
    html += '.</div>';

    html += '<table class="table table-sm align-middle">';
    html += '<thead><tr><th>Data</th><th>Piano</th><th>Aderenza</th><th>Consiglio</th><th></th></tr></thead><tbody>';
    for (const a of lista) {
        html += '<tr>';
        html += '<td>' + escapeHtml(a.data) + '</td>';
        html += '<td class="text-muted small">' + escapeHtml(a.nomePiano || "") + '</td>';
        html += '<td>' + a.percentuale + '%</td>';
        if (a.consiglioPro && a.consiglioPro.length > 0) {
            html += '<td><span class="badge bg-success">Nuovo</span></td>';
        } else {
            html += '<td><span class="text-muted small">—</span></td>';
        }
        html += '<td class="text-end d-flex gap-1 justify-content-end">';
        html += '<button class="btn btn-sm btn-outline-secondary" data-dettaglioAder="' + a.id + '">Dettaglio</button>';
        html += '<button class="btn btn-sm btn-outline-danger" data-elimAder="' + a.id + '">Elimina</button>';
        html += '</td>';
        html += '</tr>';
    }
    html += '</tbody></table>';
    cont.innerHTML = html;

    const mappaAderenze = {};
    for (const a of lista) mappaAderenze[a.id] = a;

    cont.querySelectorAll("button[data-dettaglioAder]").forEach(function (btn) {
        btn.addEventListener("click", function () {
            apriDettaglioAderenza(mappaAderenze[parseInt(btn.getAttribute("data-dettaglioAder"))]);
        });
    });

    cont.querySelectorAll("button[data-elimAder]").forEach(function (btn) {
        btn.addEventListener("click", async function () {
            if (!confirm("Eliminare questa registrazione?")) return;
            await apiDelete("/api/aderenza/" + parseInt(btn.getAttribute("data-elimAder")));
            await caricaAderenze();
        });
    });
}

function apriDettaglioAderenza(a) {
    let html = '<dl class="row mb-3">';
    html += '<dt class="col-5">Data</dt><dd class="col-7">' + escapeHtml(a.data) + '</dd>';
    html += '<dt class="col-5">Piano</dt><dd class="col-7">' + escapeHtml(a.nomePiano || "—") + '</dd>';
    html += '<dt class="col-5">Aderenza</dt><dd class="col-7"><strong>' + a.percentuale + '%</strong></dd>';
    html += '<dt class="col-5">Note</dt><dd class="col-7">' + escapeHtml(a.note || "—") + '</dd>';
    html += '</dl>';

    if (a.consiglioPro && a.consiglioPro.length > 0) {
        html += '<div class="alert alert-success">';
        html += '<strong>Consiglio del nutrizionista:</strong><br>' + escapeHtml(a.consiglioPro);
        html += '</div>';
    } else {
        html += '<p class="text-muted small">Il nutrizionista non ha ancora aggiunto un consiglio per questa giornata.</p>';
    }

    document.getElementById("modalCorpoAderenza").innerHTML = html;
    modalAderenza.show();
}

function apriFormAderenza() {
    if (!utenteCorrente.idPianoCorrente) {
        alert("Seleziona prima un piano nutrizionale dal catalogo.");
        return;
    }
    const oggi = new Date().toISOString().split("T")[0];
    document.getElementById("aderenzaData").value = oggi;
    document.getElementById("aderenzaPercentuale").value = 100;
    document.getElementById("aderenzaValoreLabel").textContent = "100";
    document.getElementById("aderenzaNote").value = "";
    document.getElementById("messaggioAderenza").textContent = "";
    document.getElementById("cardFormAderenza").classList.remove("d-none");
}

function chiudiFormAderenza() {
    document.getElementById("cardFormAderenza").classList.add("d-none");
}

async function salvaAderenza(evento) {
    evento.preventDefault();
    const dati = {
        idPiano: utenteCorrente.idPianoCorrente,
        data: document.getElementById("aderenzaData").value,
        percentuale: parseInt(document.getElementById("aderenzaPercentuale").value),
        note: document.getElementById("aderenzaNote").value.trim()
    };
    const r = await apiPost("/api/aderenza", dati);
    if (!r.ok) {
        document.getElementById("messaggioAderenza").textContent = r.dati.errore || "Errore";
        return;
    }
    chiudiFormAderenza();
    await caricaAderenze();
}

// ----- Helper -----

function escapeHtml(s) {
    return String(s == null ? "" : s)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;");
}

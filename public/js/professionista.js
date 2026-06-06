// professionista.js - Dashboard professionista: attivita', sessioni/aderenze clienti, feedback.

let modalSessione = null;
let modalAderenza = null;

window.addEventListener("DOMContentLoaded", async function () {
    const utente = await richiediUtente("professionista");
    if (!utente) return;

    document.getElementById("nomeUtente").textContent = utente.nome;
    document.getElementById("specializzazione").textContent = utente.specializzazione || "";
    document.getElementById("btnLogout").addEventListener("click", logout);

    modalSessione = new bootstrap.Modal(document.getElementById("modalDettaglioSessione"));
    modalAderenza = new bootstrap.Modal(document.getElementById("modalDettaglioAderenza"));

    // Visibilita' sezioni in base alla specializzazione.
    const spec = utente.specializzazione || "entrambi";
    const mostraProgram = (spec === "trainer" || spec === "entrambi");
    const mostraPiano   = (spec === "nutrizionista" || spec === "entrambi");

    if (mostraProgram) {
        document.getElementById("linkProgram").classList.remove("d-none");
    }
    if (mostraPiano) {
        document.getElementById("linkPiani").classList.remove("d-none");
    }
    if (!mostraProgram) {
        document.getElementById("sezioneProgram").classList.add("d-none");
    }
    if (mostraPiano) {
        document.getElementById("sezionePiano").classList.remove("d-none");
    }

    if (mostraProgram) {
        await caricaAttivita();
        await caricaSessioniClienti();
        await caricaFeedback();
    }
    if (mostraPiano) {
        await caricaAttivitaPiani();
        await caricaAderenzeClienti();
        await caricaFeedbackPiani();
    }
});

// =================== ATTIVITA' AGGREGATE ===================

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
    html += '<thead><tr><th>Programma</th><th class="text-end">Sessioni</th>';
    html += '<th class="text-end">Completate</th><th class="text-end">Adesione</th>';
    html += '<th>Ultima sessione</th></tr></thead><tbody>';
    for (const r of righe) {
        const adesione = (r.totale > 0)
            ? Math.round((r.completate / r.totale) * 100) + "%"
            : "—";
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

// =================== SESSIONI INDIVIDUALI ===================

async function caricaSessioniClienti() {
    const cont = document.getElementById("elencoSessioniClienti");
    cont.innerHTML = '<p class="text-muted">Caricamento...</p>';

    const r = await apiGet("/api/professionista/sessioni");
    if (!r.ok) {
        cont.innerHTML = '<p class="text-danger">Errore caricamento.</p>';
        return;
    }

    const sessioni = r.dati.sessioni;
    if (sessioni.length === 0) {
        cont.innerHTML = '<p class="text-muted mb-0">Nessuna sessione registrata sui tuoi programmi.</p>';
        return;
    }

    let html = '<table class="table table-sm align-middle">';
    html += '<thead><tr><th>Data</th><th>Cliente</th><th>Programma</th>';
    html += '<th>Durata</th><th>Stato</th><th></th></tr></thead><tbody>';
    for (const s of sessioni) {
        html += '<tr>';
        html += '<td>' + escapeHtml(s.data) + '</td>';
        html += '<td>' + escapeHtml(s.nomeCliente) + '</td>';
        html += '<td class="text-muted small">' + escapeHtml(s.nomeProgramma) + '</td>';
        html += '<td>' + s.durataMin + ' min</td>';
        if (s.completata) {
            html += '<td><span class="badge bg-success">Completata</span></td>';
        } else {
            html += '<td><span class="badge bg-warning text-dark">Interrotta</span></td>';
        }
        html += '<td class="text-end"><button class="btn btn-sm btn-outline-secondary" ';
        html += 'data-idx="' + sessioni.indexOf(s) + '">Dettaglio</button></td>';
        html += '</tr>';
    }
    html += '</tbody></table>';
    cont.innerHTML = html;

    // Mappa indice -> oggetto sessione per il modal.
    cont.querySelectorAll("button[data-idx]").forEach(function (btn) {
        btn.addEventListener("click", function () {
            apriDettaglioSessione(sessioni[parseInt(btn.getAttribute("data-idx"))]);
        });
    });
}

async function apriDettaglioSessione(s) {
    const obiettivoLeggibile = {
        perdita_peso: "Perdita peso",
        massa: "Aumento massa",
        mantenimento: "Mantenimento"
    };

    // ---- Sezione sessione ----
    let html = '<h6 class="text-muted mb-2">Sessione</h6>';
    html += '<dl class="row mb-3">';
    html += '<dt class="col-4">Data</dt><dd class="col-8">' + escapeHtml(s.data) + '</dd>';
    html += '<dt class="col-4">Programma</dt><dd class="col-8">' + escapeHtml(s.nomeProgramma) + '</dd>';
    html += '<dt class="col-4">Durata</dt><dd class="col-8">' + s.durataMin + ' min</dd>';
    html += '<dt class="col-4">Stato</dt><dd class="col-8">';
    html += s.completata
        ? '<span class="badge bg-success">Completata</span>'
        : '<span class="badge bg-warning text-dark">Interrotta</span>';
    html += '</dd>';
    html += '<dt class="col-4">Note</dt><dd class="col-8">' + escapeHtml(s.note || "—") + '</dd>';
    html += '</dl>';

    // ---- Esercizi del programma ----
    html += '<h6 class="text-muted mb-2">Esercizi del programma</h6>';
    const r = await apiGet("/api/programmi/" + s.idProgramma);
    if (!r.ok || r.dati.esercizi.length === 0) {
        html += '<p class="text-muted small mb-3">Nessun esercizio definito nel programma.</p>';
    } else {
        html += '<table class="table table-sm mb-3">';
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

    // ---- Bottone info cliente (nascosto di default) ----
    html += '<button type="button" id="btnToggleCliente" class="btn btn-sm btn-outline-secondary mb-3">';
    html += 'Mostra info cliente</button>';
    html += '<div id="infoClienteDiv" class="d-none border rounded p-3 bg-light">';
    html += '<h6 class="text-muted mb-2">Profilo cliente</h6>';
    html += '<dl class="row mb-0">';
    html += '<dt class="col-5">Nome</dt><dd class="col-7">' + escapeHtml(s.nomeCliente) + '</dd>';
    html += '<dt class="col-5">Email</dt><dd class="col-7">' + escapeHtml(s.emailCliente) + '</dd>';
    html += '<dt class="col-5">Eta\'</dt><dd class="col-7">' + (s.etaCliente || '—') + ' anni</dd>';
    html += '<dt class="col-5">Peso</dt><dd class="col-7">' + (s.pesoCliente || '—') + ' kg</dd>';
    html += '<dt class="col-5">Altezza</dt><dd class="col-7">' + (s.altezzaCliente || '—') + ' cm</dd>';
    html += '<dt class="col-5">Obiettivo</dt><dd class="col-7">';
    html += escapeHtml(obiettivoLeggibile[s.obiettivoCliente] || s.obiettivoCliente || '—');
    html += '</dd>';
    html += '</dl>';
    html += '</div>';

    document.getElementById("modalCorpoSessione").innerHTML = html;

    document.getElementById("btnToggleCliente").addEventListener("click", function () {
        const div = document.getElementById("infoClienteDiv");
        const nascosto = div.classList.contains("d-none");
        div.classList.toggle("d-none", !nascosto);
        this.textContent = nascosto ? "Nascondi info cliente" : "Mostra info cliente";
    });

    modalSessione.show();
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

// =================== SEZIONE NUTRIZIONISTA ===================

// ---- Statistiche aggregate piani ----

async function caricaAttivitaPiani() {
    const cont = document.getElementById("elencoAttivitaPiani");
    cont.innerHTML = '<p class="text-muted">Caricamento...</p>';

    const r = await apiGet("/api/professionista/attivita-piani");
    if (!r.ok) {
        cont.innerHTML = '<p class="text-danger">Errore caricamento.</p>';
        return;
    }

    const righe = r.dati.attivita;
    if (righe.length === 0) {
        cont.innerHTML = '<p class="text-muted mb-0">Nessun piano creato.</p>';
        return;
    }

    let html = '<table class="table table-sm align-middle">';
    html += '<thead><tr><th>Piano</th><th class="text-end">Registrazioni</th>';
    html += '<th class="text-end">Aderenza media</th><th>Ultima registrazione</th></tr></thead><tbody>';
    for (const a of righe) {
        html += '<tr>';
        html += '<td>' + escapeHtml(a.nomePiano) + '</td>';
        html += '<td class="text-end">' + a.totale + '</td>';
        html += '<td class="text-end">' + (a.totale > 0 ? a.mediaPercentuale + "%" : "—") + '</td>';
        html += '<td>' + escapeHtml(a.ultimaData || "—") + '</td>';
        html += '</tr>';
    }
    html += '</tbody></table>';
    cont.innerHTML = html;
}

// ---- Aderenze individuali ----

async function caricaAderenzeClienti() {
    const cont = document.getElementById("elencoAderenzeClienti");
    cont.innerHTML = '<p class="text-muted">Caricamento...</p>';

    const r = await apiGet("/api/professionista/aderenze");
    if (!r.ok) {
        cont.innerHTML = '<p class="text-danger">Errore caricamento.</p>';
        return;
    }

    const aderenze = r.dati.aderenze;
    if (aderenze.length === 0) {
        cont.innerHTML = '<p class="text-muted mb-0">Nessuna aderenza registrata sui tuoi piani.</p>';
        return;
    }

    let html = '<table class="table table-sm align-middle">';
    html += '<thead><tr><th>Data</th><th>Cliente</th><th>Piano</th>';
    html += '<th>Aderenza</th><th>Consiglio</th><th></th></tr></thead><tbody>';
    for (const a of aderenze) {
        html += '<tr>';
        html += '<td>' + escapeHtml(a.data) + '</td>';
        html += '<td>' + escapeHtml(a.nomeCliente) + '</td>';
        html += '<td class="text-muted small">' + escapeHtml(a.nomePiano) + '</td>';
        html += '<td>' + a.percentuale + '%</td>';
        if (a.consiglioPro && a.consiglioPro.length > 0) {
            html += '<td><span class="badge bg-success">Aggiunto</span></td>';
        } else {
            html += '<td><span class="badge bg-light text-muted border">—</span></td>';
        }
        html += '<td class="text-end"><button class="btn btn-sm btn-outline-secondary" ';
        html += 'data-idxAder="' + aderenze.indexOf(a) + '">Dettaglio</button></td>';
        html += '</tr>';
    }
    html += '</tbody></table>';
    cont.innerHTML = html;

    cont.querySelectorAll("button[data-idxAder]").forEach(function (btn) {
        btn.addEventListener("click", function () {
            apriDettaglioAderenza(aderenze[parseInt(btn.getAttribute("data-idxAder"))]);
        });
    });
}

async function apriDettaglioAderenza(a) {
    let html = '<h6 class="text-muted mb-2">Dati registrazione</h6>';
    html += '<dl class="row mb-3">';
    html += '<dt class="col-4">Data</dt><dd class="col-8">' + escapeHtml(a.data) + '</dd>';
    html += '<dt class="col-4">Piano</dt><dd class="col-8">' + escapeHtml(a.nomePiano) + '</dd>';
    html += '<dt class="col-4">Aderenza</dt><dd class="col-8"><strong>' + a.percentuale + '%</strong></dd>';
    html += '<dt class="col-4">Note cliente</dt><dd class="col-8">' + escapeHtml(a.note || "—") + '</dd>';
    html += '</dl>';

    // Consiglio attuale
    html += '<h6 class="text-muted mb-2">Il tuo consiglio</h6>';
    if (a.consiglioPro && a.consiglioPro.length > 0) {
        html += '<div class="alert alert-success py-2 small mb-3">' + escapeHtml(a.consiglioPro) + '</div>';
    } else {
        html += '<p class="text-muted small mb-2">Nessun consiglio inserito per questa giornata.</p>';
    }

    // Form per aggiungere/modificare il consiglio
    html += '<div class="mb-3">';
    html += '<label class="form-label small">Aggiungi / modifica consiglio:</label>';
    html += '<textarea id="inputConsiglio" class="form-control form-control-sm" rows="3" maxlength="300">';
    html += escapeHtml(a.consiglioPro || "");
    html += '</textarea>';
    html += '</div>';
    html += '<button id="btnSalvaConsiglio" class="btn btn-sm btn-primary me-2">Salva consiglio</button>';
    html += '<p id="erroreConsiglio" class="text-danger small mt-2 mb-2"></p>';

    // Info cliente (toggle)
    html += '<button type="button" id="btnToggleCliente" class="btn btn-sm btn-outline-secondary mb-3">Mostra info cliente</button>';
    html += '<div id="infoClienteDiv" class="d-none border rounded p-3 bg-light">';
    html += '<h6 class="text-muted mb-2">Profilo cliente</h6>';
    html += '<dl class="row mb-0">';
    html += '<dt class="col-5">Nome</dt><dd class="col-7">' + escapeHtml(a.nomeCliente) + '</dd>';
    html += '<dt class="col-5">Email</dt><dd class="col-7">' + escapeHtml(a.emailCliente) + '</dd>';
    html += '<dt class="col-5">Eta\'</dt><dd class="col-7">' + (a.etaCliente || '—') + ' anni</dd>';
    html += '<dt class="col-5">Peso</dt><dd class="col-7">' + (a.pesoCliente || '—') + ' kg</dd>';
    html += '<dt class="col-5">Altezza</dt><dd class="col-7">' + (a.altezzaCliente || '—') + ' cm</dd>';
    html += '</dl></div>';

    document.getElementById("modalCorpoAderenza").innerHTML = html;

    document.getElementById("btnToggleCliente").addEventListener("click", function () {
        const div = document.getElementById("infoClienteDiv");
        const nascosto = div.classList.contains("d-none");
        div.classList.toggle("d-none", !nascosto);
        this.textContent = nascosto ? "Nascondi info cliente" : "Mostra info cliente";
    });

    document.getElementById("btnSalvaConsiglio").addEventListener("click", async function () {
        const consiglio = document.getElementById("inputConsiglio").value.trim();
        const r = await apiPut("/api/professionista/aderenza/" + a.id + "/consiglio",
                               { consiglio: consiglio });
        if (!r.ok) {
            document.getElementById("erroreConsiglio").textContent =
                r.dati.errore || "Errore salvataggio";
            return;
        }
        // Aggiorno il dato locale e ricarico la lista.
        a.consiglioPro = consiglio;
        document.getElementById("erroreConsiglio").textContent = "";
        document.getElementById("erroreConsiglio").className = "text-success small mt-2 mb-2";
        document.getElementById("erroreConsiglio").textContent = "Consiglio salvato.";
        await caricaAderenzeClienti();
    });

    modalAderenza.show();
}

// ---- Feedback ricevuti sui piani ----

async function caricaFeedbackPiani() {
    const cont = document.getElementById("elencoFeedbackPiani");
    cont.innerHTML = '<p class="text-muted">Caricamento...</p>';

    const r = await apiGet("/api/professionista/feedback-piani");
    if (!r.ok) {
        cont.innerHTML = '<p class="text-danger">Errore caricamento.</p>';
        return;
    }

    const elenco = r.dati.feedback;
    if (elenco.length === 0) {
        cont.innerHTML = '<p class="text-muted mb-0">Nessun feedback ricevuto sui piani.</p>';
        return;
    }

    let html = '<table class="table table-sm align-middle">';
    html += '<thead><tr><th>Piano</th><th>Cliente</th><th>Voto</th><th>Commento</th><th>Data</th></tr></thead><tbody>';
    for (const f of elenco) {
        html += '<tr>';
        html += '<td>' + escapeHtml(f.nomePiano) + '</td>';
        html += '<td>' + escapeHtml(f.nomeCliente) + '</td>';
        html += '<td>' + stelle(f.voto) + '</td>';
        html += '<td>' + escapeHtml(f.commento || "") + '</td>';
        html += '<td>' + escapeHtml(f.data) + '</td>';
        html += '</tr>';
    }
    html += '</tbody></table>';
    cont.innerHTML = html;
}

// ----- Helper -----

function escapeHtml(s) {
    return String(s == null ? "" : s)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;");
}

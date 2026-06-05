// cliente.js - Dashboard cliente: programma corrente, piano nutrizionale, cataloghi.

let utenteCorrente = null;

window.addEventListener("DOMContentLoaded", async function () {
    utenteCorrente = await richiediUtente("cliente");
    if (!utenteCorrente) return;

    document.getElementById("nomeUtente").textContent = utenteCorrente.nome;
    document.getElementById("btnLogout").addEventListener("click", logout);
    document.getElementById("btnApplicaFiltri").addEventListener("click", caricaCatalogo);
    document.getElementById("btnApplicaFiltriPiani").addEventListener("click", caricaCatalogoPiani);
    document.getElementById("btnNuovaSessione").addEventListener("click", apriFormSessione);
    document.getElementById("btnAnnullaSessione").addEventListener("click", chiudiFormSessione);
    document.getElementById("formSessione").addEventListener("submit", salvaSessione);

    await caricaProgrammaCorrente();
    await caricaPianoCorrente();
    await caricaSessioni();
    await caricaCatalogo();
    await caricaCatalogoPiani();
});

// ----- Programma corrente -----

async function caricaProgrammaCorrente() {
    const cont = document.getElementById("contenutoProgrammaCorrente");
    const id = utenteCorrente.idProgrammaCorrente;
    if (!id) {
        cont.innerHTML = '<p class="text-muted mb-0">Nessun programma selezionato. Scegli un programma dal catalogo.</p>';
        return;
    }

    const r = await apiGet("/api/programmi/" + id);
    if (!r.ok) {
        cont.innerHTML = '<p class="text-danger mb-0">Errore: programma non trovato.</p>';
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
        await caricaProgrammaCorrente();
        await caricaCatalogo();
    });

    await caricaFeedbackProgramma(idProgramma);
}

// Mostra il feedback gia' lasciato dal cliente sul programma corrente,
// oppure il form per lasciarne uno nuovo.
async function caricaFeedbackProgramma(idProgramma) {
    const zona = document.getElementById("zonaFeedback");
    if (!zona) return;

    const r = await apiGet("/api/feedback/programma/" + idProgramma);
    if (!r.ok) { zona.innerHTML = ""; return; }

    const f = r.dati.feedback;
    if (f) {
        // Feedback gia' lasciato.
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
        s += (i <= voto) ? "★" : "☆";   // stella piena / vuota
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
        cont.innerHTML = '<p class="text-muted mb-0">Nessun piano selezionato. Scegli un piano dal catalogo.</p>';
        return;
    }

    const r = await apiGet("/api/piani/" + id);
    if (!r.ok) {
        cont.innerHTML = '<p class="text-danger mb-0">Errore: piano non trovato.</p>';
        return;
    }

    const p = r.dati.piano;
    const pasti = r.dati.pasti;

    let html = '<h3 class="h6 mb-1">' + escapeHtml(p.nome) + '</h3>';
    html += '<p class="small text-muted mb-2">Calorie giornaliere: ' + p.calorieGiornaliere + '</p>';

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

    html += '<button id="btnRimuoviPiano" class="btn btn-sm btn-outline-danger">Rimuovi selezione</button>';
    cont.innerHTML = html;

    document.getElementById("btnRimuoviPiano").addEventListener("click", async function () {
        await apiPost("/api/cliente/piano", { idPiano: 0 });
        utenteCorrente.idPianoCorrente = 0;
        await caricaPianoCorrente();
        await caricaCatalogoPiani();
    });
}

// ----- Catalogo programmi -----

async function caricaCatalogo() {
    const cont = document.getElementById("elencoCatalogo");
    cont.innerHTML = '<p class="text-muted">Caricamento...</p>';

    const obiettivo = document.getElementById("filtroObiettivo").value;
    const difficolta = document.getElementById("filtroDifficolta").value;
    const durataMax = document.getElementById("filtroDurataMax").value;

    let url = "/api/programmi";
    const parametri = [];
    if (obiettivo) parametri.push("obiettivo=" + encodeURIComponent(obiettivo));
    if (difficolta) parametri.push("difficolta=" + encodeURIComponent(difficolta));
    if (durataMax) parametri.push("durataMax=" + encodeURIComponent(durataMax));
    if (parametri.length > 0) url += "?" + parametri.join("&");

    const r = await apiGet(url);
    if (!r.ok) {
        cont.innerHTML = '<p class="text-danger">Errore caricamento catalogo.</p>';
        return;
    }

    const programmi = r.dati.programmi;
    if (programmi.length === 0) {
        cont.innerHTML = '<p class="text-muted">Nessun programma corrisponde ai filtri.</p>';
        return;
    }

    let html = '<table class="table table-sm align-middle">';
    html += '<thead><tr><th>Nome</th><th>Obiettivo</th><th>Difficolta\'</th><th>Durata</th><th></th></tr></thead><tbody>';
    for (const p of programmi) {
        const giaSelezionato = (p.id === utenteCorrente.idProgrammaCorrente);
        html += '<tr>';
        html += '<td>' + escapeHtml(p.nome) + '</td>';
        html += '<td>' + escapeHtml(p.obiettivo) + '</td>';
        html += '<td>' + escapeHtml(p.difficolta) + '</td>';
        html += '<td>' + p.durataSettimane + ' sett.</td>';
        html += '<td class="text-end">';
        if (giaSelezionato) {
            html += '<span class="badge bg-success">Selezionato</span>';
        } else {
            html += '<button class="btn btn-sm btn-primary" data-programma="' + p.id + '">Seleziona</button>';
        }
        html += '</td>';
        html += '</tr>';
    }
    html += '</tbody></table>';
    cont.innerHTML = html;

    cont.querySelectorAll("button[data-programma]").forEach(function (btn) {
        btn.addEventListener("click", async function () {
            const id = parseInt(btn.getAttribute("data-programma"));
            await apiPost("/api/cliente/programma", { idProgramma: id });
            utenteCorrente.idProgrammaCorrente = id;
            await caricaProgrammaCorrente();
            await caricaCatalogo();
        });
    });
}

// ----- Catalogo piani -----

async function caricaCatalogoPiani() {
    const cont = document.getElementById("elencoCatalogoPiani");
    cont.innerHTML = '<p class="text-muted">Caricamento...</p>';

    const calorieMax = document.getElementById("filtroCalorieMax").value;
    let url = "/api/piani";
    if (calorieMax) url += "?calorieMax=" + encodeURIComponent(calorieMax);

    const r = await apiGet(url);
    if (!r.ok) {
        cont.innerHTML = '<p class="text-danger">Errore caricamento catalogo.</p>';
        return;
    }

    const piani = r.dati.piani;
    if (piani.length === 0) {
        cont.innerHTML = '<p class="text-muted">Nessun piano corrisponde ai filtri.</p>';
        return;
    }

    let html = '<table class="table table-sm align-middle">';
    html += '<thead><tr><th>Nome</th><th>Calorie giornaliere</th><th></th></tr></thead><tbody>';
    for (const p of piani) {
        const giaSelezionato = (p.id === utenteCorrente.idPianoCorrente);
        html += '<tr>';
        html += '<td>' + escapeHtml(p.nome) + '</td>';
        html += '<td>' + p.calorieGiornaliere + '</td>';
        html += '<td class="text-end">';
        if (giaSelezionato) {
            html += '<span class="badge bg-success">Selezionato</span>';
        } else {
            html += '<button class="btn btn-sm btn-primary" data-piano="' + p.id + '">Seleziona</button>';
        }
        html += '</td>';
        html += '</tr>';
    }
    html += '</tbody></table>';
    cont.innerHTML = html;

    cont.querySelectorAll("button[data-piano]").forEach(function (btn) {
        btn.addEventListener("click", async function () {
            const id = parseInt(btn.getAttribute("data-piano"));
            await apiPost("/api/cliente/piano", { idPiano: id });
            utenteCorrente.idPianoCorrente = id;
            await caricaPianoCorrente();
            await caricaCatalogoPiani();
        });
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

    let html = '<table class="table table-sm align-middle">';
    html += '<thead><tr><th>Data</th><th>Durata</th><th>Stato</th><th>Note</th><th></th></tr></thead><tbody>';
    for (const s of sessioni) {
        html += '<tr>';
        html += '<td>' + escapeHtml(s.data) + '</td>';
        html += '<td>' + s.durataMin + ' min</td>';
        if (s.completata) {
            html += '<td><span class="badge bg-success">Completata</span></td>';
        } else {
            html += '<td><span class="badge bg-warning text-dark">Interrotta</span></td>';
        }
        html += '<td>' + escapeHtml(s.note || "") + '</td>';
        html += '<td class="text-end"><button class="btn btn-sm btn-outline-danger" data-elimSess="' + s.id + '">Elimina</button></td>';
        html += '</tr>';
    }
    html += '</tbody></table>';
    cont.innerHTML = html;

    cont.querySelectorAll("button[data-elimSess]").forEach(function (btn) {
        btn.addEventListener("click", async function () {
            if (!confirm("Eliminare questa sessione?")) return;
            const id = parseInt(btn.getAttribute("data-elimSess"));
            await apiDelete("/api/sessioni/" + id);
            await caricaSessioni();
        });
    });
}

function apriFormSessione() {
    if (!utenteCorrente.idProgrammaCorrente) {
        alert("Seleziona prima un programma dal catalogo.");
        return;
    }
    // Imposto la data odierna come default.
    const oggi = new Date().toISOString().split("T")[0];
    document.getElementById("sessioneData").value = oggi;
    document.getElementById("sessioneDurata").value = 45;
    document.getElementById("sessioneCompletata").value = "true";
    document.getElementById("sessioneNote").value = "";
    document.getElementById("messaggioSessione").textContent = "";
    document.getElementById("cardFormSessione").classList.remove("d-none");
}

function chiudiFormSessione() {
    document.getElementById("cardFormSessione").classList.add("d-none");
}

async function salvaSessione(evento) {
    evento.preventDefault();
    const dati = {
        idProgramma: utenteCorrente.idProgrammaCorrente,
        data: document.getElementById("sessioneData").value,
        durataMin: parseInt(document.getElementById("sessioneDurata").value),
        completata: (document.getElementById("sessioneCompletata").value === "true"),
        note: document.getElementById("sessioneNote").value.trim()
    };
    const r = await apiPost("/api/sessioni", dati);
    if (!r.ok) {
        document.getElementById("messaggioSessione").textContent = r.dati.errore || "Errore";
        return;
    }
    chiudiFormSessione();
    await caricaSessioni();
}

// ----- Helper -----

function escapeHtml(s) {
    return String(s == null ? "" : s)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;");
}

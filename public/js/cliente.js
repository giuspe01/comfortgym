// cliente.js - Dashboard cliente: programma corrente, piano nutrizionale, cataloghi.

let utenteCorrente = null;

window.addEventListener("DOMContentLoaded", async function () {
    utenteCorrente = await richiediUtente("cliente");
    if (!utenteCorrente) return;

    document.getElementById("nomeUtente").textContent = utenteCorrente.nome;
    document.getElementById("btnLogout").addEventListener("click", logout);
    document.getElementById("btnApplicaFiltri").addEventListener("click", caricaCatalogo);
    document.getElementById("btnApplicaFiltriPiani").addEventListener("click", caricaCatalogoPiani);

    await caricaProgrammaCorrente();
    await caricaPianoCorrente();
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

    html += '<button id="btnRimuoviProgramma" class="btn btn-sm btn-outline-danger">Rimuovi selezione</button>';
    cont.innerHTML = html;

    document.getElementById("btnRimuoviProgramma").addEventListener("click", async function () {
        await apiPost("/api/cliente/programma", { idProgramma: 0 });
        utenteCorrente.idProgrammaCorrente = 0;
        await caricaProgrammaCorrente();
        await caricaCatalogo();
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

// ----- Helper -----

function escapeHtml(s) {
    return String(s == null ? "" : s)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;");
}

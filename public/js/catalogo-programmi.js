// catalogo-programmi.js - Catalogo pubblico dei programmi di allenamento.

let utenteCorrente = null;
let modalDettaglio = null;
let idProgrammaSelezionatoModal = 0;

window.addEventListener("DOMContentLoaded", async function () {
    utenteCorrente = await richiediUtente("cliente");
    if (!utenteCorrente) return;

    modalDettaglio = new bootstrap.Modal(document.getElementById("modalDettaglio"));

    document.getElementById("btnLogout").addEventListener("click", logout);
    document.getElementById("btnApplicaFiltri").addEventListener("click", caricaCatalogo);
    document.getElementById("btnSelezionaModal").addEventListener("click", selezionaPrograma);

    await caricaCatalogo();
});

async function caricaCatalogo() {
    const griglia = document.getElementById("griglia");
    griglia.innerHTML = '<div class="col"><p class="text-muted">Caricamento...</p></div>';

    const difficolta = document.getElementById("filtroDifficolta").value;
    const durataMax = document.getElementById("filtroDurataMax").value;

    let url = "/api/programmi";
    const parametri = [];
    if (difficolta) parametri.push("difficolta=" + encodeURIComponent(difficolta));
    if (durataMax) parametri.push("durataMax=" + encodeURIComponent(durataMax));
    if (parametri.length > 0) url += "?" + parametri.join("&");

    const r = await apiGet(url);
    if (!r.ok) {
        griglia.innerHTML = '<div class="col"><p class="text-danger">Errore caricamento catalogo.</p></div>';
        return;
    }

    const programmi = r.dati.programmi;
    if (programmi.length === 0) {
        griglia.innerHTML = '<div class="col"><p class="text-muted">Nessun programma corrisponde ai filtri.</p></div>';
        return;
    }

    let html = "";
    for (const p of programmi) {
        const selezionato = (p.id === utenteCorrente.idProgrammaCorrente);

        // Immagine o placeholder colorato.
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
        html += '<p class="card-text small text-muted mb-1">';
        html += '<strong>Obiettivo:</strong> ' + escapeHtml(p.obiettivo);
        html += '</p>';
        html += '<p class="card-text small text-muted mb-2">';
        html += '<span class="badge bg-light text-dark border me-1">' + escapeHtml(p.difficolta) + '</span>';
        html += '<span class="badge bg-light text-dark border">' + p.durataSettimane + ' settimane</span>';
        html += '</p>';
        html += '</div>';
        html += '<div class="card-footer d-flex justify-content-between align-items-center">';
        if (selezionato) {
            html += '<span class="badge bg-success">Selezionato</span>';
        } else {
            html += '<span></span>';
        }
        html += '<button class="btn btn-sm btn-outline-primary" data-dettaglio="' + p.id + '">Dettaglio</button>';
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
}

async function apriDettaglio(idProgramma) {
    const r = await apiGet("/api/programmi/" + idProgramma);
    if (!r.ok) { alert("Impossibile caricare il programma"); return; }

    const p = r.dati.programma;
    const esercizi = r.dati.esercizi;

    document.getElementById("modalTitolo").textContent = p.nome;

    let corpo = '<p class="mb-1"><strong>Obiettivo:</strong> ' + escapeHtml(p.obiettivo) + '</p>';
    corpo += '<p class="mb-1"><strong>Difficolta\':</strong> ' + escapeHtml(p.difficolta) + '</p>';
    corpo += '<p class="mb-3"><strong>Durata:</strong> ' + p.durataSettimane + ' settimane</p>';

    if (esercizi.length === 0) {
        corpo += '<p class="text-muted">Questo programma non ha esercizi definiti.</p>';
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

    const btnSel = document.getElementById("btnSelezionaModal");
    if (p.id === utenteCorrente.idProgrammaCorrente) {
        btnSel.textContent = "Gia\' selezionato";
        btnSel.disabled = true;
    } else {
        btnSel.textContent = "Seleziona questo programma";
        btnSel.disabled = false;
    }
    idProgrammaSelezionatoModal = p.id;

    modalDettaglio.show();
}

async function selezionaPrograma() {
    const r = await apiPost("/api/cliente/programma", { idProgramma: idProgrammaSelezionatoModal });
    if (!r.ok) { alert("Errore selezione programma"); return; }
    utenteCorrente.idProgrammaCorrente = idProgrammaSelezionatoModal;
    modalDettaglio.hide();
    await caricaCatalogo();
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

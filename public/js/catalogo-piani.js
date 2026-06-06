// catalogo-piani.js - Catalogo pubblico dei piani nutrizionali.

let utenteCorrente = null;
let modalDettaglio = null;
let idPianoSelezionatoModal = 0;

window.addEventListener("DOMContentLoaded", async function () {
    utenteCorrente = await richiediUtente("cliente");
    if (!utenteCorrente) return;

    modalDettaglio = new bootstrap.Modal(document.getElementById("modalDettaglio"));

    document.getElementById("btnLogout").addEventListener("click", logout);
    document.getElementById("btnApplicaFiltri").addEventListener("click", caricaCatalogo);
    document.getElementById("btnSelezionaModal").addEventListener("click", selezionaPiano);

    await caricaCatalogo();
});

async function caricaCatalogo() {
    const griglia = document.getElementById("griglia");
    griglia.innerHTML = '<div class="col"><p class="text-muted">Caricamento...</p></div>';

    const calorieMax = document.getElementById("filtroCalorieMax").value;
    let url = "/api/piani";
    if (calorieMax) url += "?calorieMax=" + encodeURIComponent(calorieMax);

    const r = await apiGet(url);
    if (!r.ok) {
        griglia.innerHTML = '<div class="col"><p class="text-danger">Errore caricamento catalogo.</p></div>';
        return;
    }

    const piani = r.dati.piani;
    if (piani.length === 0) {
        griglia.innerHTML = '<div class="col"><p class="text-muted">Nessun piano corrisponde ai filtri.</p></div>';
        return;
    }

    let html = "";
    for (const p of piani) {
        const selezionato = (p.id === utenteCorrente.idPianoCorrente);

        let imgHtml;
        if (p.immagine && p.immagine.length > 0) {
            imgHtml = '<img src="' + escapeAttr(p.immagine) + '" class="card-img-top card-img-piano" alt="' + escapeAttr(p.nome) + '">';
        } else {
            imgHtml = '<div class="card-img-placeholder">&#x1F96C;</div>';
        }

        html += '<div class="col">';
        html += '<div class="card h-100 shadow-sm">';
        html += imgHtml;
        html += '<div class="card-body">';
        html += '<h5 class="card-title">' + escapeHtml(p.nome) + '</h5>';
        html += '<p class="card-text small text-muted mb-2">';
        html += '<span class="badge bg-light text-dark border">' + p.calorieGiornaliere + ' kcal/giorno</span>';
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

async function apriDettaglio(idPiano) {
    const r = await apiGet("/api/piani/" + idPiano);
    if (!r.ok) { alert("Impossibile caricare il piano"); return; }

    const p = r.dati.piano;
    const pasti = r.dati.pasti;

    document.getElementById("modalTitolo").textContent = p.nome;

    let corpo = '<p class="mb-2"><strong>Calorie giornaliere:</strong> ' + p.calorieGiornaliere + ' kcal</p>';

    if (p.consigliGiornalieri && p.consigliGiornalieri.trim().length > 0) {
        corpo += '<div class="alert alert-warning py-2 small mb-3">';
        corpo += '<strong>Consigli del nutrizionista:</strong> ' + escapeHtml(p.consigliGiornalieri);
        corpo += '</div>';
    }

    if (pasti.length === 0) {
        corpo += '<p class="text-muted">Nessun pasto definito.</p>';
    } else {
        corpo += '<h6>Composizione pasti</h6>';
        corpo += '<table class="table table-sm">';
        corpo += '<thead><tr><th>Pasto</th><th>Descrizione</th><th>Calorie</th></tr></thead><tbody>';
        for (const pa of pasti) {
            corpo += '<tr>';
            corpo += '<td class="text-capitalize">' + escapeHtml(pa.tipo) + '</td>';
            corpo += '<td>' + escapeHtml(pa.descrizione) + '</td>';
            corpo += '<td>' + pa.calorie + '</td>';
            corpo += '</tr>';
        }
        corpo += '</tbody></table>';
    }
    document.getElementById("modalCorpo").innerHTML = corpo;

    const btnSel = document.getElementById("btnSelezionaModal");
    if (p.id === utenteCorrente.idPianoCorrente) {
        btnSel.textContent = "Gia\' selezionato";
        btnSel.disabled = true;
    } else {
        btnSel.textContent = "Seleziona questo piano";
        btnSel.disabled = false;
    }
    idPianoSelezionatoModal = p.id;

    modalDettaglio.show();
}

async function selezionaPiano() {
    const r = await apiPost("/api/cliente/piano", { idPiano: idPianoSelezionatoModal });
    if (!r.ok) { alert("Errore selezione piano"); return; }
    utenteCorrente.idPianoCorrente = idPianoSelezionatoModal;
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

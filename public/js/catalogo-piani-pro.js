// catalogo-piani-pro.js - Gestione piani nutrizionali del professionista (card + CRUD).

let modalDettaglio = null;

window.addEventListener("DOMContentLoaded", async function () {
    const utente = await richiediUtente("professionista");
    if (!utente) return;

    // Solo nutrizionista e entrambi possono gestire i piani.
    const spec = utente.specializzazione || "entrambi";
    if (spec === "trainer") {
        window.location.href = "/professionista.html";
        return;
    }

    // Mostra il link ai programmi solo se il professionista ha quella specializzazione.
    if (spec === "entrambi") {
        document.getElementById("linkProgrammi").classList.remove("d-none");
    }

    modalDettaglio = new bootstrap.Modal(document.getElementById("modalDettaglio"));

    document.getElementById("btnLogout").addEventListener("click", logout);
    document.getElementById("btnNuovoPiano").addEventListener("click", apriFormNuovo);
    document.getElementById("btnAnnullaPiano").addEventListener("click", chiudiForm);
    document.getElementById("btnAggiungiPasto").addEventListener("click", function () {
        aggiungiRiga();
    });
    document.getElementById("formPiano").addEventListener("submit", salvaPiano);

    await caricaGriglia();
});

// ----- Griglia card -----

async function caricaGriglia() {
    const griglia = document.getElementById("griglia");
    griglia.innerHTML = '<div class="col"><p class="text-muted">Caricamento...</p></div>';

    const r = await apiGet("/api/professionista/piani");
    if (!r.ok) {
        griglia.innerHTML = '<div class="col"><p class="text-danger">Errore caricamento.</p></div>';
        return;
    }

    const piani = r.dati.piani;
    if (piani.length === 0) {
        griglia.innerHTML = '<div class="col"><p class="text-muted">Non hai ancora creato piani.</p></div>';
        return;
    }

    let html = "";
    for (const p of piani) {
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
        html += '<p class="card-text small text-muted mb-0">';
        html += '<span class="badge bg-light text-dark border">' + p.calorieGiornaliere + ' kcal/giorno</span>';
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
            eliminaPiano(parseInt(btn.getAttribute("data-elimina")));
        });
    });
}

// ----- Modal dettaglio -----

async function apriDettaglio(id) {
    const r = await apiGet("/api/piani/" + id);
    if (!r.ok) { alert("Impossibile caricare il piano"); return; }

    const p = r.dati.piano;
    const pasti = r.dati.pasti;

    document.getElementById("modalTitolo").textContent = p.nome;

    let corpo = '<p class="mb-2"><strong>Calorie giornaliere:</strong> ' + p.calorieGiornaliere + ' kcal</p>';

    if (p.consigliGiornalieri && p.consigliGiornalieri.trim().length > 0) {
        corpo += '<div class="alert alert-warning py-2 small mb-3">';
        corpo += '<strong>Consigli:</strong> ' + escapeHtml(p.consigliGiornalieri);
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
    modalDettaglio.show();
}

// ----- Form crea/modifica -----

function apriFormNuovo() {
    document.getElementById("titoloFormPiano").textContent = "Nuovo piano";
    document.getElementById("idPiano").value = "";
    document.getElementById("immaginePrecedente").value = "";
    document.getElementById("nomePiano").value = "";
    document.getElementById("caloriePiano").value = "2000";
    document.getElementById("consigliPiano").value = "";
    document.getElementById("immaginePiano").value = "";
    document.getElementById("anteprimaImmagine").classList.add("d-none");
    document.getElementById("righePasti").innerHTML = "";
    aggiungiRiga();
    nascondiMessaggio();
    document.getElementById("cardFormPiano").classList.remove("d-none");
    document.getElementById("cardFormPiano").scrollIntoView({ behavior: "smooth" });
}

async function apriFormModifica(id) {
    const r = await apiGet("/api/piani/" + id);
    if (!r.ok) { alert("Impossibile caricare il piano"); return; }

    const p = r.dati.piano;
    const pasti = r.dati.pasti;

    document.getElementById("titoloFormPiano").textContent = "Modifica piano";
    document.getElementById("idPiano").value = p.id;
    document.getElementById("immaginePrecedente").value = p.immagine || "";
    document.getElementById("nomePiano").value = p.nome;
    document.getElementById("caloriePiano").value = p.calorieGiornaliere;
    document.getElementById("consigliPiano").value = p.consigliGiornalieri || "";
    document.getElementById("immaginePiano").value = "";

    if (p.immagine && p.immagine.length > 0) {
        document.getElementById("anteprimaImmagine").classList.remove("d-none");
        document.getElementById("imgAttuale").src = p.immagine;
    } else {
        document.getElementById("anteprimaImmagine").classList.add("d-none");
    }

    document.getElementById("righePasti").innerHTML = "";
    for (const pa of pasti) aggiungiRiga(pa);
    if (pasti.length === 0) aggiungiRiga();
    nascondiMessaggio();
    document.getElementById("cardFormPiano").classList.remove("d-none");
    document.getElementById("cardFormPiano").scrollIntoView({ behavior: "smooth" });
}

function chiudiForm() {
    document.getElementById("cardFormPiano").classList.add("d-none");
}

function aggiungiRiga(pa) {
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

    // Upload immagine se l'utente ne ha selezionata una nuova.
    let urlImmagine = document.getElementById("immaginePrecedente").value;
    const fileInput = document.getElementById("immaginePiano");
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
        nome: document.getElementById("nomePiano").value.trim(),
        calorieGiornaliere: parseInt(document.getElementById("caloriePiano").value),
        consigliGiornalieri: document.getElementById("consigliPiano").value.trim(),
        immagine: urlImmagine,
        pasti: leggiPasti()
    };

    const id = document.getElementById("idPiano").value;
    const risposta = id
        ? await apiPut("/api/piani/" + id, dati)
        : await apiPost("/api/piani", dati);

    if (!risposta.ok) {
        mostraMessaggio(risposta.dati.errore || "Errore salvataggio", "danger");
        return;
    }
    chiudiForm();
    await caricaGriglia();
}

function leggiPasti() {
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
    if (!confirm("Eliminare definitivamente questo piano? Verranno cancellate anche le registrazioni di aderenza dei clienti.")) return;
    const r = await apiDelete("/api/piani/" + id);
    if (!r.ok) { alert(r.dati.errore || "Errore eliminazione"); return; }
    await caricaGriglia();
}

// ----- Helper messaggi form -----

function mostraMessaggio(testo, tipo) {
    const div = document.getElementById("messaggioFormPiano");
    div.textContent = testo;
    div.className = "alert alert-" + tipo;
}

function nascondiMessaggio() {
    const div = document.getElementById("messaggioFormPiano");
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

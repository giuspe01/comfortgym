// api.js - Helper per parlare con il backend.
//
// Tutte le rotte del nostro backend sotto /api accettano e rispondono in JSON.
// Invece di scrivere ogni volta "fetch(..., { method, headers, body, ... })",
// usiamo queste due funzioni che fanno la stessa cosa con un nome chiaro.
//
// Concetti usati:
//   - "fetch": la funzione standard di JavaScript per fare richieste HTTP.
//     Ritorna una Promise (un oggetto che "promette" un valore in futuro).
//   - "async/await": modo moderno di scrivere codice asincrono. La parola
//     'await' aspetta che la Promise si risolva, restituendo il valore.
//   - "JSON.stringify(obj)": converte un oggetto JavaScript in stringa JSON.
//   - "risposta.json()": parsifica il corpo della risposta come JSON.

// Esegue una POST verso il percorso indicato, mandando 'dati' come corpo JSON.
// Ritorna un oggetto { ok, status, dati }:
//   - ok: true se la risposta HTTP e' 2xx.
//   - status: codice numerico HTTP (200, 400, 401, ...).
//   - dati: il JSON ricevuto (oggetto JavaScript).
async function apiPost(percorso, dati) {
    const risposta = await fetch(percorso, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(dati),
        credentials: "same-origin"  // include i cookie nelle richieste
    });
    const corpo = await risposta.json();
    return { ok: risposta.ok, status: risposta.status, dati: corpo };
}

// Esegue una GET verso il percorso indicato.
async function apiGet(percorso) {
    const risposta = await fetch(percorso, {
        method: "GET",
        credentials: "same-origin"
    });
    const corpo = await risposta.json();
    return { ok: risposta.ok, status: risposta.status, dati: corpo };
}

// PUT con corpo JSON (per le modifiche).
async function apiPut(percorso, dati) {
    const risposta = await fetch(percorso, {
        method: "PUT",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(dati),
        credentials: "same-origin"
    });
    const corpo = await risposta.json();
    return { ok: risposta.ok, status: risposta.status, dati: corpo };
}

// DELETE senza corpo.
async function apiDelete(percorso) {
    const risposta = await fetch(percorso, {
        method: "DELETE",
        credentials: "same-origin"
    });
    const corpo = await risposta.json();
    return { ok: risposta.ok, status: risposta.status, dati: corpo };
}

// Mostra un messaggio nel riquadro #messaggio della pagina.
// 'tipo' puo' essere 'success', 'danger', 'warning', 'info'
// (corrispondono alle classi alert-* di Bootstrap).
function mostraMessaggio(testo, tipo) {
    const div = document.getElementById("messaggio");
    div.textContent = testo;
    div.className = "alert alert-" + tipo;  // toglie la classe d-none
}

// Nasconde il riquadro dei messaggi.
function nascondiMessaggio() {
    const div = document.getElementById("messaggio");
    if (!div) return;
    div.textContent = "";
    div.className = "alert d-none";
}

// Chiama /api/me. Se non sei autenticato, redirect a "/".
// Se 'ruoloAtteso' e' indicato e l'utente ha un altro ruolo, redirect alla
// dashboard giusta. Ritorna l'oggetto utente o null se e' avvenuto un redirect.
async function richiediUtente(ruoloAtteso) {
    const r = await apiGet("/api/me");
    if (!r.ok) {
        window.location.href = "/";
        return null;
    }
    const utente = r.dati.utente;
    if (ruoloAtteso && utente.tipo !== ruoloAtteso) {
        window.location.href = (utente.tipo === "cliente")
            ? "/cliente.html"
            : "/professionista.html";
        return null;
    }
    return utente;
}

// Esegue il logout e torna alla home.
async function logout() {
    await apiPost("/api/auth/logout", {});
    window.location.href = "/";
}

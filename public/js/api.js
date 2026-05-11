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
    div.textContent = "";
    div.className = "alert d-none";
}

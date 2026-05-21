// professionista.js - Dashboard del professionista.
//
// All'avvio verifica la sessione, controlla il ruolo, mostra nome e
// specializzazione. Le sezioni vere arrivano in seguito.

window.addEventListener("DOMContentLoaded", async function () {
    const utente = await richiediUtente("professionista");
    if (!utente) return;

    document.getElementById("nomeUtente").textContent = utente.nome;
    document.getElementById("specializzazione").textContent = utente.specializzazione || "";

    document.getElementById("btnLogout").addEventListener("click", logout);
});

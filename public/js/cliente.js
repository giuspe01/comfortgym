// cliente.js - Dashboard del cliente.
//
// All'avvio verifica la sessione, controlla il ruolo, mostra il nome utente.
// Le sezioni vere (programmi, piano, sessioni, feedback) arrivano in seguito.

window.addEventListener("DOMContentLoaded", async function () {
    const utente = await richiediUtente("cliente");
    if (!utente) return;

    document.getElementById("nomeUtente").textContent = utente.nome;

    document.getElementById("btnLogout").addEventListener("click", logout);
});

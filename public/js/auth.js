// auth.js - Pagina di accesso/registrazione.
//
// Se l'utente e' gia' loggato, redirect alla dashboard.
// Altrimenti: toggle dei due form e submit verso /api/auth/...

// Se c'e' gia' una sessione valida, salta direttamente alla dashboard.
window.addEventListener("DOMContentLoaded", async function () {
    const r = await apiGet("/api/me");
    if (r.ok && r.dati.utente) {
        const tipo = r.dati.utente.tipo;
        window.location.href = (tipo === "cliente") ? "/cliente.html" : "/professionista.html";
    }
});

// Mostra/nasconde i campi specifici al cambiare del tipo di account.
document.getElementById("regTipo").addEventListener("change", function () {
    const tipo = this.value;
    document.getElementById("campiCliente").classList.toggle("d-none", tipo !== "cliente");
    document.getElementById("campiProfessionista").classList.toggle("d-none", tipo !== "professionista");
});

// Mostra solo uno dei due form, nasconde l'altro.
// Aggiorna anche lo stile dei bottoni in alto (quale tab e' attiva).
function mostraForm(quale) {
    const formAccedi = document.getElementById("formAccedi");
    const formRegistrati = document.getElementById("formRegistrati");
    const btnAccedi = document.getElementById("btnTabAccedi");
    const btnRegistrati = document.getElementById("btnTabRegistrati");

    if (quale === "accedi") {
        formAccedi.classList.remove("d-none");
        formRegistrati.classList.add("d-none");
        btnAccedi.className = "btn btn-primary";
        btnRegistrati.className = "btn btn-outline-primary";
    } else {
        formAccedi.classList.add("d-none");
        formRegistrati.classList.remove("d-none");
        btnAccedi.className = "btn btn-outline-primary";
        btnRegistrati.className = "btn btn-primary";
    }
    nascondiMessaggio();
}

// Submit del form di accesso.
// 'evento.preventDefault()' impedisce al browser di ricaricare la pagina
// (comportamento di default del submit di un form).
document.getElementById("formAccedi").addEventListener("submit", async function (evento) {
    evento.preventDefault();

    const dati = {
        username: document.getElementById("accediUsername").value,
        password: document.getElementById("accediPassword").value
    };

    const r = await apiPost("/api/auth/login", dati);
    if (!r.ok) {
        mostraMessaggio(r.dati.errore || "Errore durante l'accesso", "danger");
        return;
    }

    // Login riuscito: il cookie di sessione e' gia' stato impostato dal server.
    // Reindirizziamo alla dashboard giusta in base al tipo di utente.
    const tipo = r.dati.utente.tipo;
    if (tipo === "cliente") {
        window.location.href = "/cliente.html";
    } else if (tipo === "professionista") {
        window.location.href = "/professionista.html";
    } else {
        mostraMessaggio("Tipo utente sconosciuto: " + tipo, "danger");
    }
});

// Submit del form di registrazione.
document.getElementById("formRegistrati").addEventListener("submit", async function (evento) {
    evento.preventDefault();

    const tipo = document.getElementById("regTipo").value;
    const dati = {
        tipo: tipo,
        nome: document.getElementById("regNome").value,
        cognome: document.getElementById("regCognome").value,
        email: document.getElementById("regEmail").value,
        username: document.getElementById("regUsername").value,
        password: document.getElementById("regPassword").value
    };

    // Campi specifici per ruolo.
    if (tipo === "cliente") {
        dati.eta = parseInt(document.getElementById("regEta").value);
        dati.peso = parseFloat(document.getElementById("regPeso").value);
        dati.altezza = parseFloat(document.getElementById("regAltezza").value);
        dati.obiettivo = document.getElementById("regObiettivo").value;
    } else {
        dati.specializzazione = document.getElementById("regSpecializzazione").value;
    }

    const r = await apiPost("/api/auth/register", dati);
    if (!r.ok) {
        mostraMessaggio(r.dati.errore || "Errore durante la registrazione", "danger");
        return;
    }

    // Registrazione riuscita: torno al form di login con un messaggio di successo
    // e precompilo lo username.
    mostraForm("accedi");
    document.getElementById("accediUsername").value = dati.username;
    document.getElementById("accediPassword").value = "";
    mostraMessaggio(r.dati.messaggio || "Registrazione completata. Ora puoi accedere.", "success");
});

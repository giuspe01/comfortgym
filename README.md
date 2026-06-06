# Palestra Digitale

Piattaforma web completa per la gestione di programmi di allenamento e piani nutrizionali, pensata per trainer e nutrizionisti che vogliono monitorare i progressi dei loro clienti online.

**Caso di studio** del corso ITPS (Ingegneria del Testing, dei Processi e del Software) — Università degli Studi di Bari, A.A. 2025-2026.

## Descrizione del progetto

"Palestra Digitale" è una soluzione end-to-end che permette a:

- **Clienti**: selezionare programmi di allenamento e piani nutrizionali, registrare sessioni di allenamento e aderenze giornaliere, lasciare feedback, consultare consigli personalizzati.
- **Professionisti** (Trainer e Nutrizionisti): creare e gestire contenuti, visualizzare statistiche di attività dei clienti, leggere feedback, aggiungere consigli personalizzati.

## Funzionalità principali

### Per i clienti
- Registrazione e login con profilo completo (età, peso, altezza, obiettivo)
- Catalogo programmi (filtrabili per obiettivo e difficoltà)
- Catalogo piani nutrizionali (filtrabili per calorie)
- Selezione di programma e piano corrente
- Registrazione sessioni di allenamento (data, durata, esercizi completati)
- Registrazione aderenza giornaliera al piano nutrizionale (percentuale)
- Visualizzazione consiglio personalizzato del nutrizionista su ogni giornata
- Feedback su programmi e piani
- Modifica del profilo (età, peso, altezza)

### Per i trainer
- Creazione, modifica e cancellazione dei programmi di allenamento
- Aggiunta di esercizi (nome, serie, ripetizioni, riposo, note)
- Upload immagini di copertina
- Dashboard con statistiche (totale sessioni, completate, ultima data)
- Lista sessioni clienti con dettagli e info anagrafiche
- Lettura feedback ricevuti sui programmi

### Per i nutrizionisti
- Creazione, modifica e cancellazione dei piani nutrizionali
- Aggiunta di pasti (tipo, descrizione, calorie)
- Upload immagini di copertina
- Dashboard con statistiche aggregate per piano
- Lista aderenze clienti con dettagli
- Aggiunta di consigli personalizzati su ogni giornata
- Lettura feedback ricevuti sui piani

### Sicurezza
- Autenticazione con hash password (djb2)
- Sessioni tramite token in cookie `HttpOnly` e `SameSite=Lax`
- Guard di autorizzazione a livello frontend e backend
- Separazione per specializzazione (trainer ↔ programmi, nutrizionista ↔ piani)
- Validazione input lato server
- Protezione SQL injection (prepared statement)
- Protezione XSS (escape HTML)
- Dati anagrafici clienti visibili solo ai professionisti

## Tecnologie

### Backend
- **Linguaggio**: C++17
- **Server HTTP**: [cpp-httplib](https://github.com/yhirose/cpp-httplib) v0.18.3
- **JSON**: [nlohmann/json](https://github.com/nlohmann/json) v3.11.3
- **Database**: SQLite 3.47 (amalgamation)

### Frontend
- **Markup**: HTML5
- **Stile**: Bootstrap 5.3.2 (CDN)
- **JavaScript**: Vanilla ES6+ (`async/await`, `fetch`)

### Build
- **Compilatore**: GCC (mingw64 su Windows) o Clang
- **Build**: Makefile cross-platform (Windows/Linux/macOS)

**Nota**: Tutte le dipendenze C/C++ sono incluse in `third_party/` — il progetto è auto-contenuto, non richiede installazioni esterne.

## Requisiti di sistema

| Strumento | Versione minima |
|---|---|
| GCC / Clang | C++17 |
| Make | 3.81+ |
| Git | qualunque |

Per eseguire il binario compilato non è richiesto nulla (linkato staticamente).

## Setup

### Windows (PowerShell)

Se non hai GCC/Make, installa con [scoop](https://scoop.sh):
```powershell
scoop install gcc make git
```

### Linux (Debian/Ubuntu)

```bash
sudo apt update && sudo apt install -y build-essential git
```

### macOS

```bash
xcode-select --install
brew install make
```

## Compilazione e avvio

```bash
# Clona il repository
git clone https://github.com/TUO_USERNAME/palestra-digitale.git
cd palestra-digitale

# Compila
make

# Avvia il server
./palestra          # Linux/macOS
palestra.exe        # Windows
```

Apri il browser su **[http://localhost:8080](http://localhost:8080)**.

**Output atteso:**
```
Database aperto: data/palestra.db
Palestra digitale - server in ascolto su http://localhost:8080
```

### Comandi Make utili

```bash
make clean      # Rimuove gli oggetti compilati e l'eseguibile
make rebuild    # Ricompila da zero
make run        # Compila e avvia il server direttamente
```

## Primo accesso

1. Apri [http://localhost:8080](http://localhost:8080)
2. Clicca "Registrazione"
3. Scegli il tipo di utente:
   - **Cliente**: compila età, peso, altezza, obiettivo di fitness
   - **Professionista**: scegli specializzazione (Trainer, Nutrizionista, Entrambi)
4. Compila username, password, nome, cognome, email
5. Clicca "Registrati"
6. Accedi con le tue credenziali

## Struttura del progetto

```
.
├── Makefile                      # Build cross-platform
├── README.md                     # Questo file
├── docs/
│   └── documentazione.md         # Analisi, progettazione, test completi
├── third_party/                  # Dipendenze (single-header/source)
│   ├── httplib.h                 # cpp-httplib v0.18.3
│   ├── json.hpp                  # nlohmann/json v3.11.3
│   ├── sqlite3.c / sqlite3.h     # SQLite 3.47 amalgamation
│   └── sqlite3ext.h
├── public/                       # Frontend statico
│   ├── index.html                # Home (login/registrazione)
│   ├── cliente.html              # Dashboard cliente
│   ├── professionista.html       # Dashboard professionista
│   ├── catalogo-programmi.html
│   ├── catalogo-piani.html
│   ├── catalogo-programmi-pro.html
│   ├── catalogo-piani-pro.html
│   ├── css/stile.css
│   ├── js/                       # JavaScript
│   └── uploads/                  # Immagini caricate (creata al primo avvio)
├── src/
│   ├── main.cpp                  # Entry point del server
│   ├── modello/                  # Classi dominio (Utente, Programma, Piano, ecc.)
│   ├── servizi/                  # Business logic (Auth, ServizioProgrammi, ecc.)
│   ├── controller/               # Route handler HTTP
│   ├── persistenza/              # Wrapper SQLite
│   └── utils/                    # Utility (Hash, Validazione, HttpUtils)
└── data/                         # Database SQLite (creato al primo avvio)
```

## Database

Il file `data/palestra.db` viene creato automaticamente al primo avvio. Contiene 11 tabelle:

- **utenti** — clienti e professionisti
- **programmi** — programmi di allenamento
- **esercizi** — esercizi per programma
- **piani** — piani nutrizionali
- **pasti** — pasti per piano
- **sessioni** — sessioni registrate dai clienti
- **feedback** — feedback su programmi
- **feedback_piani** — feedback su piani
- **aderenza_nutrizionale** — aderenze giornaliere al piano (con consiglio del nutrizionista)
- **sessioni_web** — token di sessione per login
- **allegati** — riservato per sviluppi futuri

**Per azzerare il database**: elimina `data/palestra.db` e riavvia il server.

## Architettura

Il progetto segue un'architettura a tre strati:

1. **Modello** (`src/modello/`) — Classi del dominio (Utente, Cliente, Professionista, Programma, ecc.)
2. **Servizi** (`src/servizi/`) — Business logic e accesso al DB (Auth.cpp, ServizioProgrammi.cpp, ecc.)
3. **Controller** (`src/controller/`) — Rotte HTTP REST (AuthController.cpp, ProgrammiController.cpp, ecc.)

I dati vengono serializzati in JSON tramite il metodo `toJson()` e trasferiti tra browser e server via REST API.

## Documentazione tecnica

Per una documentazione completa consulta **[docs/documentazione.md](docs/documentazione.md)**:
- Analisi delle specifiche (48 requisiti funzionali)
- Progettazione dell'architettura
- Schema del database
- Librerie e dipendenze
- Frammenti di codice rappresentativi
- Esiti dei test funzionali (83 test case)

## Punti di forza

- **Copertura completa**: 48 requisiti funzionali implementati e testati
- **Architettura modulare**: facile da estendere e manutenere
- **Sicurezza di base**: hash password, sessioni sicure, validazione input, protezione XSS/SQL injection
- **Persistenza affidabile**: transazioni, vincoli referenziali, cleanup coerente
- **Portabilità**: auto-contenuto, nessuna dipendenza esterna
- **Interfaccia chiara**: bootstrap, pagine distinte per ruoli, modal per dettagli

## Aree di miglioramento

- Hash password: sostituire djb2 con bcrypt/argon2
- Token sessione: usare `/dev/urandom` (Unix) o `CryptGenRandom` (Windows) invece di `rand()`
- Validazione email: estendere al formato RFC 5322
- Concorrenza: considerare modalità WAL per SQLite sotto carico
- Buffer fissi: introdurre contenitori dinamici per volumi elevati

## Sviluppi futuri

- Assegnazione esplicita di professionista a cliente
- Reset password via email
- Filtri avanzati sul catalogo
- Statistiche con grafici (Chart.js)
- Notifiche all'utente
- Applicazione mobile / Progressive Web App

## Licenza

Progetto didattico. Le librerie in `third_party/` mantengono le rispettive licenze:
- cpp-httplib: MIT
- nlohmann/json: MIT
- SQLite: Public Domain

## Contatti e supporto

Per domande o issues, consulta la documentazione in `docs/` o apri un issue su GitHub.
